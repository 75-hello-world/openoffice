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

#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/cropprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive2d
	{
		Primitive2DSequence GraphicPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& 
#ifdef USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
            rViewInformation
#else
            /*rViewInformation*/
#endif // USE_DEBUG_CODE_TO_TEST_METAFILE_DECOMPOSE
            ) const
		{
			Primitive2DSequence aRetval;

			if(255L != getGraphicAttr().GetTransparency())
			{
                // do not apply mirroring from GraphicAttr to the Metafile by calling
                // GetTransformedGraphic, this will try to mirror the Metafile using Scale()
                // at the Metafile. This again calls Scale at the single MetaFile actions,
                // but this implementation never worked. I reworked that implementations,
                // but for security reasons i will try not to use it.
                basegfx::B2DHomMatrix aTransform(getTransform());

                if(getGraphicAttr().IsMirrored())
                {
                    // content needs mirroring
                    const bool bHMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_HORZ);
                    const bool bVMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_VERT);

                    // mirror by applying negative scale to the unit primitive and
                    // applying the object transformation on it.
                    aTransform = basegfx::tools::createScaleB2DHomMatrix(
                        bHMirr ? -1.0 : 1.0,
                        bVMirr ? -1.0 : 1.0);
                    aTransform.translate(
                        bHMirr ? 1.0 : 0.0,
                        bVMirr ? 1.0 : 0.0);
                    aTransform = getTransform() * aTransform;
                }

		        // Get transformed graphic. Suppress rotation and cropping, only filtering is needed
		        // here (and may be replaced later on). Cropping is handled below as mask primitive (if set).
                // Also need to suppress mirroring, it is part of the transformation now (see above).
		        GraphicAttr aSuppressGraphicAttr(getGraphicAttr());
		        aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
		        aSuppressGraphicAttr.SetRotation(0);
		        aSuppressGraphicAttr.SetMirrorFlags(0);

                const GraphicObject& rGraphicObject = getGraphicObject();
                const Graphic aTransformedGraphic(rGraphicObject.GetTransformedGraphic(&aSuppressGraphicAttr));

                aRetval = create2DDecompositionOfGraphic(
                    aTransformedGraphic,
                    aTransform);

                if(aRetval.getLength())
                {
                    // check for cropping
                    if(getGraphicAttr().IsCropped())
                    {
                        // calculate scalings between real image size and logic object size. This
                        // is necessary since the crop values are relative to original bitmap size
                        const basegfx::B2DVector aObjectScale(aTransform * basegfx::B2DVector(1.0, 1.0));
                        const basegfx::B2DVector aCropScaleFactor(
                            rGraphicObject.calculateCropScaling(
                                aObjectScale.getX(),
                                aObjectScale.getY(),
                                getGraphicAttr().GetLeftCrop(),
                                getGraphicAttr().GetTopCrop(),
                                getGraphicAttr().GetRightCrop(),
                                getGraphicAttr().GetBottomCrop()));

                        // embed content in cropPrimitive
                        Primitive2DReference xPrimitive(
                            new CropPrimitive2D(
                                aRetval,
                                aTransform,
                                getGraphicAttr().GetLeftCrop() * aCropScaleFactor.getX(),
                                getGraphicAttr().GetTopCrop() * aCropScaleFactor.getY(),
                                getGraphicAttr().GetRightCrop() * aCropScaleFactor.getX(),
                                getGraphicAttr().GetBottomCrop() * aCropScaleFactor.getY()));

                        aRetval = Primitive2DSequence(&xPrimitive, 1);
                    }
                }
			}

			return aRetval;
		}

		GraphicPrimitive2D::GraphicPrimitive2D(
			const basegfx::B2DHomMatrix& rTransform, 
			const GraphicObject& rGraphicObject,
			const GraphicAttr& rGraphicAttr)
		:	BufferedDecompositionPrimitive2D(),
			maTransform(rTransform),
			maGraphicObject(rGraphicObject),
			maGraphicAttr(rGraphicAttr)
		{
		}

		GraphicPrimitive2D::GraphicPrimitive2D(
			const basegfx::B2DHomMatrix& rTransform, 
			const GraphicObject& rGraphicObject)
		:	BufferedDecompositionPrimitive2D(),
			maTransform(rTransform),
			maGraphicObject(rGraphicObject),
			maGraphicAttr()
		{
		}

		basegfx::B2DRange GraphicPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
		{
			return getTransform() * basegfx::B2DRange::getUnitB2DRange();
		}

		// provide unique ID
		ImplPrimitrive2DIDBlock(GraphicPrimitive2D, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D)

	} // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
