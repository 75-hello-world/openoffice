/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <vclhelperbitmaprender.hxx>
#include <svtools/grfmgr.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>
#include <vcl/outdev.hxx>
#include <vclhelperbitmaptransform.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

//////////////////////////////////////////////////////////////////////////////
// support for different kinds of bitmap rendering using vcl

namespace drawinglayer
{
	void RenderBitmapPrimitive2D_GraphicManager(
		OutputDevice& rOutDev, 
		const BitmapEx& rBitmapEx, 
		const basegfx::B2DHomMatrix& rTransform)
	{
		// prepare attributes
		GraphicAttr aAttributes;

		// decompose matrix to check for shear, rotate and mirroring
		basegfx::B2DVector aScale;
		basegfx::B2DPoint aTranslate;
		double fRotate, fShearX;
		rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

		// mirror flags
		aAttributes.SetMirrorFlags(
			(basegfx::fTools::less(aScale.getX(), 0.0) ? BMP_MIRROR_HORZ : 0)|
			(basegfx::fTools::less(aScale.getY(), 0.0) ? BMP_MIRROR_VERT : 0));

		// rotation
		if(!basegfx::fTools::equalZero(fRotate))
		{
			double fRotation(fmod(3600.0 - (fRotate * (10.0 / F_PI180)), 3600.0));
			aAttributes.SetRotation((sal_uInt16)(fRotation));
		}

		// prepare Bitmap
		basegfx::B2DRange aOutlineRange(basegfx::B2DRange::getUnitB2DRange());

		if(basegfx::fTools::equalZero(fRotate))
		{
			aOutlineRange.transform(rTransform);
		}
		else
		{
			// if rotated, create the unrotated output rectangle for the GraphicManager paint
			const basegfx::B2DHomMatrix aSimpleObjectMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
				fabs(aScale.getX()), fabs(aScale.getY()),
				aTranslate.getX(), aTranslate.getY()));

			aOutlineRange.transform(aSimpleObjectMatrix);
		}

		// prepare dest coordinates
		const Point	aPoint(
                basegfx::fround(aOutlineRange.getMinX()), 
                basegfx::fround(aOutlineRange.getMinY()));
        const Size aSize(
                basegfx::fround(aOutlineRange.getWidth()),
                basegfx::fround(aOutlineRange.getHeight()));

		// paint it using GraphicManager
		Graphic aGraphic(rBitmapEx);
		GraphicObject aGraphicObject(aGraphic);
		aGraphicObject.Draw(&rOutDev, aPoint, aSize, &aAttributes);
	}

	void RenderBitmapPrimitive2D_BitmapEx(
		OutputDevice& rOutDev, 
		const BitmapEx& rBitmapEx, 
		const basegfx::B2DHomMatrix& rTransform)
	{
		// only translate and scale, use vcl's DrawBitmapEx().
		BitmapEx aContent(rBitmapEx);

		// prepare dest coor. Necessary to expand since vcl's DrawBitmapEx draws one pix less
		const basegfx::B2DRange aOutlineRange(rTransform * basegfx::B2DRange::getUnitB2DRange());

		// prepare dest coordinates
		const Point	aPoint(
                basegfx::fround(aOutlineRange.getMinX()), 
                basegfx::fround(aOutlineRange.getMinY()));
        const Size aSize(
                basegfx::fround(aOutlineRange.getWidth()),
                basegfx::fround(aOutlineRange.getHeight()));

		// decompose matrix to check for shear, rotate and mirroring
		basegfx::B2DVector aScale;
		basegfx::B2DPoint aTranslate;
		double fRotate, fShearX;
		rTransform.decompose(aScale, aTranslate, fRotate, fShearX);

		// Check mirroring.
		sal_uInt32 nMirrorFlags(BMP_MIRROR_NONE);

		if(basegfx::fTools::less(aScale.getX(), 0.0))
		{
			nMirrorFlags |= BMP_MIRROR_HORZ;
		}

		if(basegfx::fTools::less(aScale.getY(), 0.0))
		{
			nMirrorFlags |= BMP_MIRROR_VERT;
		}

		if(BMP_MIRROR_NONE != nMirrorFlags)
		{
			aContent.Mirror(nMirrorFlags);
		}

		// draw bitmap
		rOutDev.DrawBitmapEx(aPoint, aSize, aContent);
	}

	void RenderBitmapPrimitive2D_self(
		OutputDevice& rOutDev, 
		const BitmapEx& rBitmapEx, 
		const basegfx::B2DHomMatrix& rTransform)
	{
		// process self with free transformation (containing shear and rotate). Get dest rect in pixels.
		const basegfx::B2DRange aOutlineRange(rTransform * basegfx::B2DRange::getUnitB2DRange());
		const Rectangle aDestRectLogic(
			basegfx::fround(aOutlineRange.getMinX()), 
			basegfx::fround(aOutlineRange.getMinY()),
			basegfx::fround(aOutlineRange.getMaxX()),
			basegfx::fround(aOutlineRange.getMaxY()));
		const Rectangle aDestRectPixel(rOutDev.LogicToPixel(aDestRectLogic));

		// #i96708# check if Metafile is recorded
		const GDIMetaFile* pMetaFile = rOutDev.GetConnectMetaFile();
		const bool bRecordToMetaFile(pMetaFile && pMetaFile->IsRecord() && !pMetaFile->IsPause());

		// intersect with output pixel size, but only
		// when not recording to metafile
		const Rectangle aOutputRectPixel(Point(), rOutDev.GetOutputSizePixel());
		Rectangle aCroppedRectPixel(bRecordToMetaFile ? aDestRectPixel : aDestRectPixel.GetIntersection(aOutputRectPixel));

		if(!aCroppedRectPixel.IsEmpty())
		{
			// as maximum for destination, orientate at aOutputRectPixel, but
			// take a rotation of 45 degrees (sqrt(2)) as maximum expansion into account
			const Size aSourceSizePixel(rBitmapEx.GetSizePixel());
			const double fMaximumArea(
				(double)aOutputRectPixel.getWidth() * 
				(double)aOutputRectPixel.getHeight() *
				1.4142136); // 1.4142136 taken as sqrt(2.0)

			// test if discrete view size (pixel) maybe too big and limit it
			const double fArea(aCroppedRectPixel.getWidth() * aCroppedRectPixel.getHeight());
			const bool bNeedToReduce(fArea > fMaximumArea);
			double fReduceFactor(1.0);
			const Size aDestSizePixel(aCroppedRectPixel.GetSize());

			if(bNeedToReduce)
			{
				fReduceFactor = sqrt(fMaximumArea / fArea);
				aCroppedRectPixel.setWidth(basegfx::fround(aCroppedRectPixel.getWidth() * fReduceFactor));
				aCroppedRectPixel.setHeight(basegfx::fround(aCroppedRectPixel.getHeight() * fReduceFactor));
			}
			
			// build transform from pixel in aDestination to pixel in rBitmapEx
			// from relative in aCroppedRectPixel to relative in aDestRectPixel
			// No need to take bNeedToReduce into account, TopLeft is unchanged
            basegfx::B2DHomMatrix aTransform(basegfx::tools::createTranslateB2DHomMatrix(
                aCroppedRectPixel.Left() - aDestRectPixel.Left(), 
                aCroppedRectPixel.Top() - aDestRectPixel.Top()));

			// from relative in aDestRectPixel to absolute Logic. Here it
			// is essential to adapt to reduce factor (if used)
			double fAdaptedDRPWidth((double)aDestRectPixel.getWidth());
			double fAdaptedDRPHeight((double)aDestRectPixel.getHeight());

			if(bNeedToReduce)
			{
				fAdaptedDRPWidth *= fReduceFactor;
				fAdaptedDRPHeight *= fReduceFactor;
			}

			aTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(
				aDestRectLogic.getWidth() / fAdaptedDRPWidth, aDestRectLogic.getHeight() / fAdaptedDRPHeight,
				aDestRectLogic.Left(), aDestRectLogic.Top())
				* aTransform;

			// from absolute in Logic to unified object coordinates (0.0 .. 1.0 in x and y)
			basegfx::B2DHomMatrix aInvBitmapTransform(rTransform);
			aInvBitmapTransform.invert();
			aTransform = aInvBitmapTransform * aTransform;

			// from unit object coordinates to rBitmapEx pixel coordintes
			aTransform.scale(aSourceSizePixel.getWidth() - 1L, aSourceSizePixel.getHeight() - 1L);

			// create bitmap using source, destination and linear back-transformation
			BitmapEx aDestination = impTransformBitmapEx(rBitmapEx, aCroppedRectPixel, aTransform);

			// paint
			if(bNeedToReduce)
			{
				// paint in target size
				if(bRecordToMetaFile)
				{
					rOutDev.DrawBitmapEx(
						rOutDev.PixelToLogic(aCroppedRectPixel.TopLeft()), 
						rOutDev.PixelToLogic(aDestSizePixel),
						aDestination);
				}
				else
				{
					const bool bWasEnabled(rOutDev.IsMapModeEnabled());
					rOutDev.EnableMapMode(false);
					
					rOutDev.DrawBitmapEx(
						aCroppedRectPixel.TopLeft(), 
						aDestSizePixel,
						aDestination);

					rOutDev.EnableMapMode(bWasEnabled);
				}
			}
			else
			{
				if(bRecordToMetaFile)
				{
					rOutDev.DrawBitmapEx(
						rOutDev.PixelToLogic(aCroppedRectPixel.TopLeft()), 
						aDestination);
				}
				else
				{
					const bool bWasEnabled(rOutDev.IsMapModeEnabled());
					rOutDev.EnableMapMode(false);

					rOutDev.DrawBitmapEx(
						aCroppedRectPixel.TopLeft(), 
						aDestination);

					rOutDev.EnableMapMode(bWasEnabled);
				}
			}
		}
	}
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
