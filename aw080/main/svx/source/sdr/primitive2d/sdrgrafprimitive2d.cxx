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



#include "precompiled_svx.hxx"
#include <svx/sdr/primitive2d/sdrgrafprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/primitive2d/sdrdecompositiontools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/sdrdecompositiontools2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
	namespace primitive2d
	{
		Primitive2DSequence SdrGrafPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
		{
			Primitive2DSequence  aRetval;

			// create unit outline polygon
			basegfx::B2DPolygon aUnitOutline(basegfx::tools::createUnitPolygon());

			// add fill, but only when graphic ist transparent
			if(!getSdrLFSTAttribute().getFill().isDefault() && isTransparent())
			{
				appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, 
					createPolyPolygonFillPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline), 
                        getTransform(), 
                        getSdrLFSTAttribute().getFill(), 
                        getSdrLFSTAttribute().getFillFloatTransGradient()));
			}

			// add line
			if(!getSdrLFSTAttribute().getLine().isDefault())
			{
				// if line width is given, polygon needs to be grown by half of it to make the
				// outline to be outside of the bitmap
				if(0.0 != getSdrLFSTAttribute().getLine().getWidth())
				{
					// decompose to get scale
					basegfx::B2DVector aScale;
					basegfx::B2DPoint aTranslate;
					double fRotate, fShearX;
					getTransform().decompose(aScale, aTranslate, fRotate, fShearX);

					// create expanded range (add relative half line width to unit rectangle)
					double fHalfLineWidth(getSdrLFSTAttribute().getLine().getWidth() * 0.5);
					double fScaleX(0.0 != aScale.getX() ? fHalfLineWidth / fabs(aScale.getX()) : 1.0);
					double fScaleY(0.0 != aScale.getY() ? fHalfLineWidth / fabs(aScale.getY()) : 1.0);
					const basegfx::B2DRange aExpandedRange(-fScaleX, -fScaleY, 1.0 + fScaleX, 1.0 + fScaleY);
					basegfx::B2DPolygon aExpandedUnitOutline(basegfx::tools::createPolygonFromRect(aExpandedRange));

					appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, 
                        createPolygonLinePrimitive(
                            aExpandedUnitOutline, 
                            getTransform(), 
                            getSdrLFSTAttribute().getLine(),
							attribute::SdrLineStartEndAttribute()));
				}
				else
				{
					appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, 
                        createPolygonLinePrimitive(
                            aUnitOutline, getTransform(), 
                            getSdrLFSTAttribute().getLine(),
							attribute::SdrLineStartEndAttribute()));
				}
			}

			// add graphic content
			if(255L != getGraphicAttr().GetTransparency())
			{
				const Primitive2DReference xGraphicContentPrimitive(
                    new GraphicPrimitive2D(
                        getTransform(), 
                        getGraphicObject(), 
                        getGraphicAttr()));

                appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xGraphicContentPrimitive);
			}

			// add text
			if(!getSdrLFSTAttribute().getText().isDefault())
			{
				appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, 
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline), 
                        getTransform(), 
						getSdrLFSTAttribute().getText(), 
                        getSdrLFSTAttribute().getLine(), 
                        false, 
                        false, 
                        false));
			}

			// add shadow
			if(!getSdrLFSTAttribute().getShadow().isDefault())
			{
                aRetval = createEmbeddedShadowPrimitive(
                    aRetval, 
                    getSdrLFSTAttribute().getShadow());
			}

			return aRetval;
		}

		SdrGrafPrimitive2D::SdrGrafPrimitive2D(
			const basegfx::B2DHomMatrix& rTransform, 
			const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
			const GraphicObject& rGraphicObject,
			const GraphicAttr& rGraphicAttr)
		:	BufferedDecompositionPrimitive2D(),
			maTransform(rTransform),
			maSdrLFSTAttribute(rSdrLFSTAttribute),
			maGraphicObject(rGraphicObject),
			maGraphicAttr(rGraphicAttr)
		{
			// reset some values from GraphicAttr which are part of transformation already
			maGraphicAttr.SetRotation(0);
			maGraphicAttr.SetMirrorFlags(0);
		}

		bool SdrGrafPrimitive2D::isTransparent() const
		{
			return ((0L != getGraphicAttr().GetTransparency()) || (getGraphicObject().IsTransparent()));
		}

		// provide unique ID
		ImplPrimitrive2DIDBlock(SdrGrafPrimitive2D, PRIMITIVE2D_ID_SDRGRAFPRIMITIVE2D)

	} // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
