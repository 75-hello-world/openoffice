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
#include "precompiled_svx.hxx"

#include <svx/svdoashp.hxx>
#include "svx/unoapi.hxx"
#include <svx/unoshape.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <unotools/datetime.hxx>
#include <sfx2/lnkbase.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XCustomShapeEngine.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include "unopolyhelper.hxx"
#include <comphelper/processfactory.hxx>
#include <svl/urihelper.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/svdogrp.hxx>
#include <vcl/salbtype.hxx>		// FRound
#include <svx/svddrag.hxx>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdattrx.hxx>  // NotPersistItems
#include <svx/svdoedge.hxx>  // #32383# Die Verbinder nach Move nochmal anbroadcasten
#include "svx/svdglob.hxx"   // StringCache
#include "svx/svdstr.hrc"    // Objektname
#include <editeng/eeitem.hxx>
#include "editeng/editstat.hxx"
#include <svx/svdoutl.hxx>
#include <editeng/outlobj.hxx>
#include <svx/sdtfchim.hxx>
#include "../svx/EnhancedCustomShapeGeometry.hxx"
#include "../svx/EnhancedCustomShapeTypeNames.hxx"
#include "../svx/EnhancedCustomShape2d.hxx"
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeAdjustmentValue.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeTextFrame.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegment.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeSegmentCommand.hpp>
#include <editeng/writingmodeitem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svxids.hrc>
#include <svl/whiter.hxx>
#include <svx/sdr/properties/customshapeproperties.hxx>
#include <svx/sdr/contact/viewcontactofsdrobjcustomshape.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <vcl/bmpacc.hxx>
#include <svx/svdview.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdlegacy.hxx>
#include <svx/sdrtexthelpers.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;

static MSO_SPT ImpGetCustomShapeType( const SdrObjCustomShape& rCustoShape )
{
	MSO_SPT eRetValue = mso_sptNil;

	rtl::OUString aEngine( ( (SfxStringItem&)rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE ) ).GetValue() );
	if ( !aEngine.getLength() || aEngine.equalsAscii( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) )
	{
		rtl::OUString sShapeType;
		const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
		SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)rCustoShape.GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
		Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
		if ( pAny && ( *pAny >>= sShapeType ) )
			eRetValue = EnhancedCustomShapeTypeNames::Get( sShapeType );
	}
	return eRetValue;
};

static bool ImpVerticalSwitch( const SdrObjCustomShape& rCustoShape )
{
	bool bRet = false;
	MSO_SPT eShapeType( ImpGetCustomShapeType( rCustoShape ) );
	switch( eShapeType )
	{
		case mso_sptAccentBorderCallout90 :		// 2 ortho
		case mso_sptBorderCallout1 :			// 2 diag
		case mso_sptBorderCallout2 :			// 3
		{
			bRet = true;
		}
		break;
/*
		case mso_sptCallout1 :
		case mso_sptAccentCallout1 :
		case mso_sptAccentBorderCallout1 :
		case mso_sptBorderCallout90 :
		case mso_sptCallout90 :
		case mso_sptAccentCallout90 :
		case mso_sptCallout2 :
		case mso_sptCallout3 :
		case mso_sptAccentCallout2 :
		case mso_sptAccentCallout3 :
		case mso_sptBorderCallout3 :
		case mso_sptAccentBorderCallout2 :
		case mso_sptAccentBorderCallout3 :
*/
		default: break;
	}
	return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i37011# create a clone with all attributes changed to shadow attributes
// and translation executed, too.
SdrObject* ImpCreateShadowObjectClone(const SdrObject& rOriginal, const SfxItemSet& rOriginalSet)
{
	SdrObject* pRetval = 0L;
	const bool bShadow(((SdrOnOffItem&)rOriginalSet.Get(SDRATTR_SHADOW)).GetValue());

	if(bShadow)
	{
		// create a shadow representing object
		const sal_Int32 nXDist(((SdrMetricItem&)(rOriginalSet.Get(SDRATTR_SHADOWXDIST))).GetValue());
		const sal_Int32 nYDist(((SdrMetricItem&)(rOriginalSet.Get(SDRATTR_SHADOWYDIST))).GetValue());
		const ::Color aShadowColor(((XColorItem&)(rOriginalSet.Get(SDRATTR_SHADOWCOLOR))).GetColorValue());
		const sal_uInt16 nShadowTransparence(((SdrPercentItem&)(rOriginalSet.Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue());
		pRetval = rOriginal.CloneSdrObject();
		DBG_ASSERT(pRetval, "ImpCreateShadowObjectClone: Could not clone object (!)");

		// look for used stuff
		SdrObjListIter aIterator(rOriginal);
		bool bLineUsed(false);
		bool bAllFillUsed(false);
		bool bSolidFillUsed(false);
		bool bGradientFillUsed(false);
		bool bHatchFillUsed(false);
		bool bBitmapFillUsed(false);

		while(aIterator.IsMore())
		{
			SdrObject* pObj = aIterator.Next();
			XFillStyle eFillStyle = ((XFillStyleItem&)(pObj->GetMergedItem(XATTR_FILLSTYLE))).GetValue();

			if(!bLineUsed)
			{
				XLineStyle eLineStyle = ((XLineStyleItem&)(pObj->GetMergedItem(XATTR_LINESTYLE))).GetValue();

				if(XLINE_NONE != eLineStyle)
				{
					bLineUsed = true;
				}
			}

			if(!bAllFillUsed)
			{
				if(!bSolidFillUsed && XFILL_SOLID == eFillStyle)
				{
					bSolidFillUsed = true;
					bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
				}
				if(!bGradientFillUsed && XFILL_GRADIENT == eFillStyle)
				{
					bGradientFillUsed = true;
					bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
				}
				if(!bHatchFillUsed && XFILL_HATCH == eFillStyle)
				{
					bHatchFillUsed = true;
					bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
				}
				if(!bBitmapFillUsed && XFILL_BITMAP == eFillStyle)
				{
					bBitmapFillUsed = true;
					bAllFillUsed = (bSolidFillUsed || bGradientFillUsed || bHatchFillUsed || bBitmapFillUsed);
				}
			}
		}

		// translate to shadow coordinates
		sdr::legacy::MoveSdrObject(*pRetval, Size(nXDist, nYDist));

		// set items as needed
		SfxItemSet aTempSet(rOriginalSet);

		// SJ: #40108# :-(  if a SvxWritingModeItem (Top->Bottom) is set the text object
		// is creating a paraobject, but paraobjects can not be created without model. So
		// we are preventing the crash by setting the writing mode always left to right,
		// this is not bad since our shadow geometry does not contain text.
        aTempSet.Put( SvxWritingModeItem( com::sun::star::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION ) );

		// no shadow
		aTempSet.Put(SdrOnOffItem(SDRATTR_SHADOW, false));
		aTempSet.Put(SdrMetricItem(SDRATTR_SHADOWXDIST, 0L));
		aTempSet.Put(SdrMetricItem(SDRATTR_SHADOWYDIST, 0L));

		// line color and transparence like shadow
		if(bLineUsed)
		{
			aTempSet.Put(XLineColorItem(String(), aShadowColor));
			aTempSet.Put(XLineTransparenceItem(nShadowTransparence));
		}

		// fill color and transparence like shadow
		if(bSolidFillUsed)
		{
			aTempSet.Put(XFillColorItem(String(), aShadowColor));
			aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
		}

		// gradient and transparence like shadow
		if(bGradientFillUsed)
		{
			XGradient aGradient(((XFillGradientItem&)(rOriginalSet.Get(XATTR_FILLGRADIENT))).GetGradientValue());
			sal_uInt8 nStartLuminance(aGradient.GetStartColor().GetLuminance());
			sal_uInt8 nEndLuminance(aGradient.GetEndColor().GetLuminance());

			if(aGradient.GetStartIntens() != 100)
			{
				nStartLuminance = (sal_uInt8)(nStartLuminance * ((double)aGradient.GetStartIntens() / 100.0));
			}

			if(aGradient.GetEndIntens() != 100)
			{
				nEndLuminance = (sal_uInt8)(nEndLuminance * ((double)aGradient.GetEndIntens() / 100.0));
			}

            ::Color aStartColor(
				(sal_uInt8)((nStartLuminance * aShadowColor.GetRed()) / 256),
				(sal_uInt8)((nStartLuminance * aShadowColor.GetGreen()) / 256),
				(sal_uInt8)((nStartLuminance * aShadowColor.GetBlue()) / 256));

            ::Color aEndColor(
				(sal_uInt8)((nEndLuminance * aShadowColor.GetRed()) / 256),
				(sal_uInt8)((nEndLuminance * aShadowColor.GetGreen()) / 256),
				(sal_uInt8)((nEndLuminance * aShadowColor.GetBlue()) / 256));

			aGradient.SetStartColor(aStartColor);
			aGradient.SetEndColor(aEndColor);
			aTempSet.Put(XFillGradientItem(aTempSet.GetPool(), aGradient));
			aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
		}

		// hatch and transparence like shadow
		if(bHatchFillUsed)
		{
			XHatch aHatch(((XFillHatchItem&)(rOriginalSet.Get(XATTR_FILLHATCH))).GetHatchValue());
			aHatch.SetColor(aShadowColor);
			aTempSet.Put(XFillHatchItem(aTempSet.GetPool(), aHatch));
			aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
		}

		// bitmap and transparence like shadow
		if(bBitmapFillUsed)
		{
            GraphicObject aGraphicObject(((XFillBitmapItem&)(rOriginalSet.Get(XATTR_FILLBITMAP))).GetGraphicObject());
            const BitmapEx aBitmapEx(aGraphicObject.GetGraphic().GetBitmapEx());
			Bitmap aBitmap(aBitmapEx.GetBitmap());

			if(!aBitmap.IsEmpty())
			{
    			BitmapReadAccess* pReadAccess = aBitmap.AcquireReadAccess();

                if(pReadAccess)
				{
					Bitmap aDestBitmap(aBitmap.GetSizePixel(), 24L);
					BitmapWriteAccess* pWriteAccess = aDestBitmap.AcquireWriteAccess();

					if(pWriteAccess)
					{
						for(sal_Int32 y(0L); y < pReadAccess->Height(); y++)
						{
							for(sal_Int32 x(0L); x < pReadAccess->Width(); x++)
							{
								sal_uInt16 nLuminance((sal_uInt16)pReadAccess->GetLuminance(y, x) + 1);
								const BitmapColor aDestColor(
									(sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetRed()) >> 8L),
									(sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetGreen()) >> 8L),
									(sal_uInt8)((nLuminance * (sal_uInt16)aShadowColor.GetBlue()) >> 8L));
								pWriteAccess->SetPixel(y, x, aDestColor);
							}
						}

						aDestBitmap.ReleaseAccess(pWriteAccess);
					}

					aBitmap.ReleaseAccess(pReadAccess);

                    if(aBitmapEx.IsTransparent())
                    {
                        if(aBitmapEx.IsAlpha())
                        {
                            aGraphicObject.SetGraphic(Graphic(BitmapEx(aDestBitmap, aBitmapEx.GetAlpha())));
                        }
                        else
                        {
                            aGraphicObject.SetGraphic(Graphic(BitmapEx(aDestBitmap, aBitmapEx.GetMask())));
                        }
                    }
                    else
                    {
                        aGraphicObject.SetGraphic(Graphic(aDestBitmap));
                    }
				}
			}

			aTempSet.Put(XFillBitmapItem(aTempSet.GetPool(), aGraphicObject));
			aTempSet.Put(XFillTransparenceItem(nShadowTransparence));
		}

		// set attributes and paint shadow object
		pRetval->SetMergedItemSet( aTempSet );
	}
	return pRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Reference< XCustomShapeEngine > SdrObjCustomShape::GetCustomShapeEngine( const SdrObjCustomShape* pCustomShape )
{
	Reference< XCustomShapeEngine > xCustomShapeEngine;
	String aEngine(((SfxStringItem&)pCustomShape->GetMergedItem( SDRATTR_CUSTOMSHAPE_ENGINE )).GetValue());
	if ( !aEngine.Len() )
		aEngine = String( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.drawing.EnhancedCustomShapeEngine" ) );

	Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

	Reference< XShape > aXShape = GetXShapeForSdrObject( (SdrObjCustomShape*)pCustomShape );
	if ( aXShape.is() )
	{
		if ( aEngine.Len() && xFactory.is() )
		{
			Sequence< Any > aArgument( 1 );
			Sequence< PropertyValue > aPropValues( 1 );
			aPropValues[ 0 ].Name = rtl::OUString::createFromAscii( "CustomShape" );
			aPropValues[ 0 ].Value <<= aXShape;
			aArgument[ 0 ] <<= aPropValues;
			Reference< XInterface > xInterface( xFactory->createInstanceWithArguments( aEngine, aArgument ) );
			if ( xInterface.is() )
				xCustomShapeEngine = Reference< XCustomShapeEngine >( xInterface, UNO_QUERY );
		}
	}
	return xCustomShapeEngine;
}
const SdrObject* SdrObjCustomShape::GetSdrObjectFromCustomShape() const
{
	if ( !mXRenderedCustomShape.is() )
	{
		Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( this ) );
		if ( xCustomShapeEngine.is() )
			((SdrObjCustomShape*)this)->mXRenderedCustomShape = xCustomShapeEngine->render();
	}
	SdrObject* pRenderedCustomShape = mXRenderedCustomShape.is()
				? GetSdrObjectFromXShape( mXRenderedCustomShape )
				: NULL;
	return pRenderedCustomShape;
}

// #i37011# Shadow geometry creation
const SdrObject* SdrObjCustomShape::GetSdrObjectShadowFromCustomShape() const
{
	if(!mpLastShadowGeometry)
	{
		const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();
		if(pSdrObject)
		{
			const SfxItemSet& rOriginalSet = GetObjectItemSet();
			const bool bShadow(((SdrOnOffItem&)rOriginalSet.Get( SDRATTR_SHADOW )).GetValue());

			if(bShadow)
			{
				// create a clone with all attributes changed to shadow attributes
				// and translation executed, too.
				((SdrObjCustomShape*)this)->mpLastShadowGeometry = ImpCreateShadowObjectClone(*pSdrObject, rOriginalSet);
			}
		}
	}

	return mpLastShadowGeometry;
}

bool SdrObjCustomShape::IsTextPath() const
{
	const rtl::OUString	sTextPath( RTL_CONSTASCII_USTRINGPARAM ( "TextPath" ) );
	bool bTextPathOn = false;
	SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
	Any* pAny = rGeometryItem.GetPropertyValueByName( sTextPath, sTextPath );
	if ( pAny )
		*pAny >>= bTextPathOn;
	return bTextPathOn;
}

bool SdrObjCustomShape::UseNoFillStyle() const
{
	bool bRet = false;
	rtl::OUString sShapeType;
	const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
	SdrCustomShapeGeometryItem& rGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	Any* pAny = rGeometryItem.GetPropertyValueByName( sType );
	if ( pAny )
		*pAny >>= sShapeType;
	bRet = IsCustomShapeFilledByDefault( EnhancedCustomShapeTypeNames::Get( sType ) ) == 0;

	return bRet;
}

bool SdrObjCustomShape::IsMirroredX() const
{
	bool bMirroredX = false;
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
	com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
	if ( pAny )
		*pAny >>= bMirroredX;
	return bMirroredX;
}
bool SdrObjCustomShape::IsMirroredY() const
{
	bool bMirroredY = false;
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	const rtl::OUString	sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
	com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
	if ( pAny )
		*pAny >>= bMirroredY;
	return bMirroredY;
}
void SdrObjCustomShape::SetMirroredX( const bool bMirrorX )
{
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
	//com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredX );
	PropertyValue aPropVal;
	aPropVal.Name = sMirroredX;
	aPropVal.Value <<= bMirrorX;
	aGeometryItem.SetPropertyValue( aPropVal );
	SetMergedItem( aGeometryItem );
}
void SdrObjCustomShape::SetMirroredY( const bool bMirrorY )
{
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	const rtl::OUString	sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
	//com::sun::star::uno::Any* pAny = aGeometryItem.GetPropertyValueByName( sMirroredY );
	PropertyValue aPropVal;
	aPropVal.Name = sMirroredY;
	aPropVal.Value <<= bMirrorY;
	aGeometryItem.SetPropertyValue( aPropVal );
	SetMergedItem( aGeometryItem );
}

double SdrObjCustomShape::GetObjectRotation() const
{
	return fObjectRotation;
}

double SdrObjCustomShape::GetExtraTextRotation() const
{
	const com::sun::star::uno::Any* pAny;
	SdrCustomShapeGeometryItem& rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
	const rtl::OUString sTextRotateAngle( RTL_CONSTASCII_USTRINGPARAM ( "TextRotateAngle" ) );
	pAny = rGeometryItem.GetPropertyValueByName( sTextRotateAngle );
	double fExtraTextRotateAngle = 0.0;
	if ( pAny )
		*pAny >>= fExtraTextRotateAngle;
	return fExtraTextRotateAngle;
}
basegfx::B2DPolyPolygon SdrObjCustomShape::GetLineGeometry( const SdrObjCustomShape* pCustomShape, const bool bBezierAllowed )
{
	basegfx::B2DPolyPolygon aRetval;
	bool bRet = false;
	Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
	if ( xCustomShapeEngine.is() )
	{
		com::sun::star::drawing::PolyPolygonBezierCoords aBezierCoords = xCustomShapeEngine->getLineGeometry();
		try
		{
			aRetval = basegfx::tools::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(aBezierCoords);
			if ( !bBezierAllowed && aRetval.areControlPointsUsed())
			{
				aRetval = basegfx::tools::adaptiveSubdivideByAngle(aRetval);
			}
			bRet = true;
		}
		catch ( const com::sun::star::lang::IllegalArgumentException )
		{
		}
	}
	return aRetval;
}

std::vector< SdrCustomShapeInteraction > SdrObjCustomShape::GetInteractionHandles( const SdrObjCustomShape* pCustomShape ) const
{
	std::vector< SdrCustomShapeInteraction > xRet;
	try
	{
		Reference< XCustomShapeEngine > xCustomShapeEngine( GetCustomShapeEngine( pCustomShape ) );
		if ( xCustomShapeEngine.is() )
		{
			int i;
			Sequence< Reference< XCustomShapeHandle > > xInteractionHandles( xCustomShapeEngine->getInteraction() );
			for ( i = 0; i < xInteractionHandles.getLength(); i++ )
			{
				if ( xInteractionHandles[ i ].is() )
				{
					SdrCustomShapeInteraction aSdrCustomShapeInteraction;
					aSdrCustomShapeInteraction.xInteraction = xInteractionHandles[ i ];
					aSdrCustomShapeInteraction.aPosition = xInteractionHandles[ i ]->getPosition();

					sal_Int32 nMode = 0;
					switch( ImpGetCustomShapeType( *this ) )
					{
						case mso_sptAccentBorderCallout90 :		// 2 ortho
						{
							if ( !i )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
							else if ( i == 1)
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE | CUSTOMSHAPE_HANDLE_ORTHO4;
						}
						break;

						case mso_sptWedgeRectCallout :
						case mso_sptWedgeRRectCallout :
						case mso_sptCloudCallout :
						case mso_sptWedgeEllipseCallout :
						{
							if ( !i )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED;
						}
						break;

						case mso_sptBorderCallout1 :			// 2 diag
						{
							if ( !i )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
							else if ( i == 1 )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE;
						}
						break;
						case mso_sptBorderCallout2 :			// 3
						{
							if ( !i )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
							else if ( i == 2 )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X | CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y | CUSTOMSHAPE_HANDLE_MOVE_SHAPE;
						}
						break;
						case mso_sptCallout90 :
						case mso_sptAccentCallout90 :
						case mso_sptBorderCallout90 :
						case mso_sptCallout1 :
						case mso_sptCallout2 :
						case mso_sptCallout3 :
						case mso_sptAccentCallout1 :
						case mso_sptAccentCallout2 :
						case mso_sptAccentCallout3 :
						case mso_sptBorderCallout3 :
						case mso_sptAccentBorderCallout1 :
						case mso_sptAccentBorderCallout2 :
						case mso_sptAccentBorderCallout3 :
						{
							if ( !i )
								nMode |= CUSTOMSHAPE_HANDLE_RESIZE_FIXED | CUSTOMSHAPE_HANDLE_CREATE_FIXED;
						}
						break;
						default: break;
					}
					aSdrCustomShapeInteraction.nMode = nMode;
					xRet.push_back( aSdrCustomShapeInteraction );
				}
			}
		}
	}
	catch( const uno::RuntimeException& )
	{
	}
	return xRet;
}

//////////////////////////////////////////////////////////////////////////////
// BaseProperties section
#define	DEFAULT_MINIMUM_SIGNED_COMPARE	((sal_Int32)0x80000000)
#define	DEFAULT_MAXIMUM_SIGNED_COMPARE	((sal_Int32)0x7fffffff)

sdr::properties::BaseProperties* SdrObjCustomShape::CreateObjectSpecificProperties()
{
	return new sdr::properties::CustomShapeProperties(*this);
}

SdrObjCustomShape::SdrObjCustomShape(SdrModel& rSdrModel) 
:	SdrTextObj(rSdrModel, basegfx::B2DHomMatrix(), OBJ_TEXT, true),
	fObjectRotation( 0.0 ),
	mpLastShadowGeometry(0)
{
}

SdrObjCustomShape::~SdrObjCustomShape()
{
	// delete buffered display geometry
	InvalidateRenderGeometry();
}

void SdrObjCustomShape::copyDataFromSdrObject(const SdrObject& rSource)
{
	if(this != &rSource)
	{
		const SdrObjCustomShape* pSource = dynamic_cast< const SdrObjCustomShape* >(&rSource);

		if(pSource)
		{
			// call parent
			SdrTextObj::copyDataFromSdrObject(rSource);

			// copy local data
			aName = pSource->aName;
			fObjectRotation = pSource->fObjectRotation;
			InvalidateRenderGeometry();
		}
		else
		{
			OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
		}
	}
}

SdrObject* SdrObjCustomShape::CloneSdrObject(SdrModel* pTargetModel) const
{
	SdrObjCustomShape* pClone = new SdrObjCustomShape(
		pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
	OSL_ENSURE(pClone, "CloneSdrObject error (!)");
	pClone->copyDataFromSdrObject(*this);

	return pClone;
}

void SdrObjCustomShape::MergeDefaultAttributes( const rtl::OUString* pType )
{
	PropertyValue aPropVal;
	rtl::OUString sShapeType;
	const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );
	if ( pType && pType->getLength() )
	{
		sal_Int32 nType = pType->toInt32();
		if ( nType )
			sShapeType = EnhancedCustomShapeTypeNames::Get( static_cast< MSO_SPT >( nType ) );
		else
			sShapeType = *pType;

		aPropVal.Name = sType;
		aPropVal.Value <<= sShapeType;
		aGeometryItem.SetPropertyValue( aPropVal );
	}
	else
	{
		Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
		if ( pAny )
			*pAny >>= sShapeType;
	}
	MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

	const sal_Int32* pDefData = NULL;
	const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
	if ( pDefCustomShape )
		pDefData = pDefCustomShape->pDefData;

	com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeAdjustmentValue > seqAdjustmentValues;

	//////////////////////
	// AdjustmentValues //
	//////////////////////
	const rtl::OUString	sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
	const Any* pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sAdjustmentValues );
	if ( pAny )
		*pAny >>= seqAdjustmentValues;
	if ( pDefCustomShape && pDefData )	// now check if we have to default some adjustment values
	{
		// first check if there are adjustment values are to be appended
		sal_Int32 i, nAdjustmentValues = seqAdjustmentValues.getLength();
		sal_Int32 nAdjustmentDefaults = *pDefData++;
		if ( nAdjustmentDefaults > nAdjustmentValues )
		{
			seqAdjustmentValues.realloc( nAdjustmentDefaults );
			for ( i = nAdjustmentValues; i < nAdjustmentDefaults; i++ )
			{
				seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
				seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;	// com::sun::star::beans::PropertyState_DEFAULT_VALUE;
			}
		}
		// check if there are defaulted adjustment values that should be filled the hard coded defaults (pDefValue)
		sal_Int32 nCount = nAdjustmentValues > nAdjustmentDefaults ? nAdjustmentDefaults : nAdjustmentValues;
		for ( i = 0; i < nCount; i++ )
		{
			if ( seqAdjustmentValues[ i ].State != com::sun::star::beans::PropertyState_DIRECT_VALUE )
			{
				seqAdjustmentValues[ i ].Value <<= pDefData[ i ];
				seqAdjustmentValues[ i ].State = com::sun::star::beans::PropertyState_DIRECT_VALUE;
			}
		}
	}
	aPropVal.Name = sAdjustmentValues;
	aPropVal.Value <<= seqAdjustmentValues;
	aGeometryItem.SetPropertyValue( aPropVal );

	///////////////
	// Coordsize //
	///////////////
	const rtl::OUString	sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
	const Any* pViewBox = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sViewBox );
	com::sun::star::awt::Rectangle aViewBox;
	if ( !pViewBox || !(*pViewBox >>= aViewBox ) )
	{
		if ( pDefCustomShape )
		{
			aViewBox.X = 0;
			aViewBox.Y = 0;
			aViewBox.Width = pDefCustomShape->nCoordWidth;
			aViewBox.Height= pDefCustomShape->nCoordHeight;
			aPropVal.Name = sViewBox;
			aPropVal.Value <<= aViewBox;
			aGeometryItem.SetPropertyValue( aPropVal );
		}
	}

	const rtl::OUString	sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );

	//////////////////////
	// Path/Coordinates //
	//////////////////////
	const rtl::OUString	sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
	{
		com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates;

		sal_Int32 i, nCount = pDefCustomShape->nVertices;
		seqCoordinates.realloc( nCount );
		for ( i = 0; i < nCount; i++ )
		{
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
		}
		aPropVal.Name = sCoordinates;
		aPropVal.Value <<= seqCoordinates;
		aGeometryItem.SetPropertyValue( sPath, aPropVal );
	}

	/////////////////////
	// Path/GluePoints //
	/////////////////////
	const rtl::OUString	sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
	{
		com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqGluePoints;
		sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
		seqGluePoints.realloc( nCount );
		for ( i = 0; i < nCount; i++ )
		{
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
		}
		aPropVal.Name = sGluePoints;
		aPropVal.Value <<= seqGluePoints;
		aGeometryItem.SetPropertyValue( sPath, aPropVal );
	}

	///////////////////
	// Path/Segments //
	///////////////////
	const rtl::OUString	sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sSegments );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
	{
		com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > seqSegments;

		sal_Int32 i, nCount = pDefCustomShape->nElements;
		seqSegments.realloc( nCount );
		for ( i = 0; i < nCount; i++ )
		{
			EnhancedCustomShapeSegment& rSegInfo = seqSegments[ i ];
			sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
			switch( nSDat >> 8 )
			{
				case 0x00 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
					rSegInfo.Count   = nSDat & 0xff;
					if ( !rSegInfo.Count )
						rSegInfo.Count = 1;
				}
				break;
				case 0x20 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
					rSegInfo.Count   = nSDat & 0xff;
					if ( !rSegInfo.Count )
						rSegInfo.Count = 1;
				}
				break;
				case 0x40 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
					rSegInfo.Count   = nSDat & 0xff;
					if ( !rSegInfo.Count )
						rSegInfo.Count = 1;
				}
				break;
				case 0x60 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
					rSegInfo.Count   = 0;
				}
				break;
				case 0x80 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
					rSegInfo.Count   = 0;
				}
				break;
				case 0xa1 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
					rSegInfo.Count   = ( nSDat & 0xff ) / 3;
				}
				break;
				case 0xa2 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
					rSegInfo.Count   = ( nSDat & 0xff ) / 3;
				}
				break;
				case 0xa3 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
					rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
				}
				break;
				case 0xa4 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
					rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
				}
				break;
				case 0xa5 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
					rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
				}
				break;
				case 0xa6 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
					rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
				}
				break;
				case 0xa7 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
					rSegInfo.Count   = nSDat & 0xff;
				}
				break;
				case 0xa8 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
					rSegInfo.Count   = nSDat & 0xff;
				}
				break;
				case 0xaa :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
					rSegInfo.Count   = 0;
				}
				break;
				case 0xab :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
					rSegInfo.Count   = 0;
				}
				break;
				default:
				case 0xf8 :
				{
					rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
					rSegInfo.Count   = nSDat;
				}
				break;
			}
		}
		aPropVal.Name = sSegments;
		aPropVal.Value <<= seqSegments;
		aGeometryItem.SetPropertyValue( sPath, aPropVal );
	}

	///////////////////
	// Path/StretchX //
	///////////////////
	const rtl::OUString	sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
	if ( !pAny && pDefCustomShape )
	{
		sal_Int32 nXRef = pDefCustomShape->nXRef;
		if ( ( nXRef != DEFAULT_MINIMUM_SIGNED_COMPARE ) )
		{
			aPropVal.Name = sStretchX;
			aPropVal.Value <<= nXRef;
			aGeometryItem.SetPropertyValue( sPath, aPropVal );
		}
	}

	///////////////////
	// Path/StretchY //
	///////////////////
	const rtl::OUString	sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
	if ( !pAny && pDefCustomShape )
	{
		sal_Int32 nYRef = pDefCustomShape->nYRef;
		if ( ( nYRef != DEFAULT_MINIMUM_SIGNED_COMPARE ) )
		{
			aPropVal.Name = sStretchY;
			aPropVal.Value <<= nYRef;
			aGeometryItem.SetPropertyValue( sPath, aPropVal );
		}
	}

	/////////////////////
	// Path/TextFrames //
	/////////////////////
	const rtl::OUString	sTextFrames( RTL_CONSTASCII_USTRINGPARAM ( "TextFrames" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
	{
		com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > seqTextFrames;

		sal_Int32 i, nCount = pDefCustomShape->nTextRect;
		seqTextFrames.realloc( nCount );
		const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
		for ( i = 0; i < nCount; i++, pRectangles++ )
		{
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.First,	  pRectangles->nPairA.nValA );
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].TopLeft.Second,	  pRectangles->nPairA.nValB );
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
			EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
		}
		aPropVal.Name = sTextFrames;
		aPropVal.Value <<= seqTextFrames;
		aGeometryItem.SetPropertyValue( sPath, aPropVal );
	}

	///////////////
	// Equations //
	///////////////
	const rtl::OUString	sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
	{
		com::sun::star::uno::Sequence< rtl::OUString > seqEquations;

		sal_Int32 i, nCount = pDefCustomShape->nCalculation;
		seqEquations.realloc( nCount );
		const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
		for ( i = 0; i < nCount; i++, pData++ )
			seqEquations[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );
		aPropVal.Name = sEquations;
		aPropVal.Value <<= seqEquations;
		aGeometryItem.SetPropertyValue( aPropVal );
	}

	/////////////
	// Handles //
	/////////////
	const rtl::OUString	sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
	pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
	if ( !pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
	{
		com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles;

		sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
		const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
		seqHandles.realloc( nCount );
		for ( i = 0; i < nCount; i++, pData++ )
		{
			sal_Int32 nPropertiesNeeded = 1;	// position is always needed
			sal_Int32 nFlags = pData->nFlags;
			if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
				nPropertiesNeeded++;
			if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
				nPropertiesNeeded++;
			if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
				nPropertiesNeeded++;
			if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
			{
				nPropertiesNeeded++;
				if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
				{
					if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
						nPropertiesNeeded++;
					if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
						nPropertiesNeeded++;
				}
			}
			else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
			{
				if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
					nPropertiesNeeded++;
				if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
					nPropertiesNeeded++;
				if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
					nPropertiesNeeded++;
				if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
					nPropertiesNeeded++;
			}

			n = 0;
			com::sun::star::beans::PropertyValues& rPropValues = seqHandles[ i ];
			rPropValues.realloc( nPropertiesNeeded );

			// POSITION
			{
				const rtl::OUString	sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
				::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
				EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
				EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
				rPropValues[ n ].Name = sPosition;
				rPropValues[ n++ ].Value <<= aPosition;
			}
			if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
			{
				const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
				bool bMirroredX = true;
				rPropValues[ n ].Name = sMirroredX;
				rPropValues[ n++ ].Value <<= bMirroredX;
			}
			if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
			{
				const rtl::OUString	sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
				bool bMirroredY = true;
				rPropValues[ n ].Name = sMirroredY;
				rPropValues[ n++ ].Value <<= bMirroredY;
			}
			if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
			{
				const rtl::OUString	sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
				bool bSwitched = true;
				rPropValues[ n ].Name = sSwitched;
				rPropValues[ n++ ].Value <<= bSwitched;
			}
			if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
			{
				const rtl::OUString	sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
				::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
				EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
					( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, true  );
				EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
					( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, false );
				rPropValues[ n ].Name = sPolar;
				rPropValues[ n++ ].Value <<= aCenter;
				if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
				{
					if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
					{
						const rtl::OUString	sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
						::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
						EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
							( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
						rPropValues[ n ].Name = sRadiusRangeMinimum;
						rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
					}
					if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
					{
						const rtl::OUString	sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
						::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
						EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
							( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
						rPropValues[ n ].Name = sRadiusRangeMaximum;
						rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
					}
				}
			}
			else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
			{
				if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
				{
					const rtl::OUString	sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
					::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
					EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
						( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
					rPropValues[ n ].Name = sRangeXMinimum;
					rPropValues[ n++ ].Value <<= aRangeXMinimum;
				}
				if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
				{
					const rtl::OUString	sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
					::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
					EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
						( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
					rPropValues[ n ].Name = sRangeXMaximum;
					rPropValues[ n++ ].Value <<= aRangeXMaximum;
				}
				if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
				{
					const rtl::OUString	sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
					::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
					EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
						( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
					rPropValues[ n ].Name = sRangeYMinimum;
					rPropValues[ n++ ].Value <<= aRangeYMinimum;
				}
				if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
				{
					const rtl::OUString	sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
					::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
					EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
						( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
					rPropValues[ n ].Name = sRangeYMaximum;
					rPropValues[ n++ ].Value <<= aRangeYMaximum;
				}
			}
		}
		aPropVal.Name = sHandles;
		aPropVal.Value <<= seqHandles;
		aGeometryItem.SetPropertyValue( aPropVal );
	}
	SetMergedItem( aGeometryItem );
}

bool SdrObjCustomShape::IsDefaultGeometry( const DefaultType eDefaultType ) const
{
	bool bIsDefaultGeometry = false;

	PropertyValue aPropVal;
	rtl::OUString sShapeType;
	const rtl::OUString	sType( RTL_CONSTASCII_USTRINGPARAM ( "Type" ) );
	SdrCustomShapeGeometryItem aGeometryItem( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) );

	Any *pAny = aGeometryItem.GetPropertyValueByName( sType );
	if ( pAny )
		*pAny >>= sShapeType;

	MSO_SPT eSpType = EnhancedCustomShapeTypeNames::Get( sShapeType );

	const mso_CustomShape* pDefCustomShape = GetCustomShapeContent( eSpType );
	const rtl::OUString	sPath( RTL_CONSTASCII_USTRINGPARAM ( "Path" ) );
	switch( eDefaultType )
	{
		case DEFAULT_VIEWBOX :
		{
			const rtl::OUString	sViewBox( RTL_CONSTASCII_USTRINGPARAM ( "ViewBox" ) );
			const Any* pViewBox = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sViewBox );
			com::sun::star::awt::Rectangle aViewBox;
			if ( pViewBox && ( *pViewBox >>= aViewBox ) )
			{
				if ( ( aViewBox.Width == pDefCustomShape->nCoordWidth )
					&& ( aViewBox.Height == pDefCustomShape->nCoordHeight ) )
					bIsDefaultGeometry = true;
			}
		}
		break;

		case DEFAULT_PATH :
		{
			const rtl::OUString	sCoordinates( RTL_CONSTASCII_USTRINGPARAM ( "Coordinates" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sCoordinates );
			if ( pAny && pDefCustomShape && pDefCustomShape->nVertices && pDefCustomShape->pVertices )
			{
				com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqCoordinates1, seqCoordinates2;
				if ( *pAny >>= seqCoordinates1 )
				{
					sal_Int32 i, nCount = pDefCustomShape->nVertices;
					seqCoordinates2.realloc( nCount );
					for ( i = 0; i < nCount; i++ )
					{
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ i ].First, pDefCustomShape->pVertices[ i ].nValA );
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqCoordinates2[ i ].Second, pDefCustomShape->pVertices[ i ].nValB );
					}
					if ( seqCoordinates1 == seqCoordinates2 )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( ( pDefCustomShape->nVertices == 0 ) || ( pDefCustomShape->pVertices == 0 ) ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_GLUEPOINTS :
		{
			const rtl::OUString	sGluePoints( RTL_CONSTASCII_USTRINGPARAM ( "GluePoints" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sGluePoints );
			if ( pAny && pDefCustomShape && pDefCustomShape->nGluePoints && pDefCustomShape->pGluePoints )
			{
				com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeParameterPair> seqGluePoints1, seqGluePoints2;
				if ( *pAny >>= seqGluePoints1 )
				{
					sal_Int32 i, nCount = pDefCustomShape->nGluePoints;
					seqGluePoints2.realloc( nCount );
					for ( i = 0; i < nCount; i++ )
					{
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints2[ i ].First, pDefCustomShape->pGluePoints[ i ].nValA );
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqGluePoints2[ i ].Second, pDefCustomShape->pGluePoints[ i ].nValB );
					}
					if ( seqGluePoints1 == seqGluePoints2 )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( pDefCustomShape->nGluePoints == 0 ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_SEGMENTS :
		{
			///////////////////
			// Path/Segments //
			///////////////////
			const rtl::OUString	sSegments( RTL_CONSTASCII_USTRINGPARAM ( "Segments" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sSegments );
			if ( pAny )
			{
				com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeSegment > seqSegments1, seqSegments2;
				if ( *pAny >>= seqSegments1 )
				{
					if ( pDefCustomShape && pDefCustomShape->nElements && pDefCustomShape->pElements )
					{
						sal_Int32 i, nCount = pDefCustomShape->nElements;
						if ( nCount )
						{
							seqSegments2.realloc( nCount );
							for ( i = 0; i < nCount; i++ )
							{
								EnhancedCustomShapeSegment& rSegInfo = seqSegments2[ i ];
								sal_uInt16 nSDat = pDefCustomShape->pElements[ i ];
								switch( nSDat >> 8 )
								{
									case 0x00 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::LINETO;
										rSegInfo.Count   = nSDat & 0xff;
										if ( !rSegInfo.Count )
											rSegInfo.Count = 1;
									}
									break;
									case 0x20 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CURVETO;
										rSegInfo.Count   = nSDat & 0xff;
										if ( !rSegInfo.Count )
											rSegInfo.Count = 1;
									}
									break;
									case 0x40 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::MOVETO;
										rSegInfo.Count   = nSDat & 0xff;
										if ( !rSegInfo.Count )
											rSegInfo.Count = 1;
									}
									break;
									case 0x60 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOSESUBPATH;
										rSegInfo.Count   = 0;
									}
									break;
									case 0x80 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ENDSUBPATH;
										rSegInfo.Count   = 0;
									}
									break;
									case 0xa1 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSETO;
										rSegInfo.Count   = ( nSDat & 0xff ) / 3;
									}
									break;
									case 0xa2 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ANGLEELLIPSE;
										rSegInfo.Count   = ( nSDat & 0xff ) / 3;
									}
									break;
									case 0xa3 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARCTO;
										rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
									}
									break;
									case 0xa4 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ARC;
										rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
									}
									break;
									case 0xa5 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARCTO;
										rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
									}
									break;
									case 0xa6 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::CLOCKWISEARC;
										rSegInfo.Count   = ( nSDat & 0xff ) >> 2;
									}
									break;
									case 0xa7 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTX;
										rSegInfo.Count   = nSDat & 0xff;
									}
									break;
									case 0xa8 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::ELLIPTICALQUADRANTY;
										rSegInfo.Count   = nSDat & 0xff;
									}
									break;
									case 0xaa :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOFILL;
										rSegInfo.Count   = 0;
									}
									break;
									case 0xab :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::NOSTROKE;
										rSegInfo.Count   = 0;
									}
									break;
									default:
									case 0xf8 :
									{
										rSegInfo.Command = EnhancedCustomShapeSegmentCommand::UNKNOWN;
										rSegInfo.Count   = nSDat;
									}
									break;
								}
							}
							if ( seqSegments1 == seqSegments2 )
								bIsDefaultGeometry = true;
						}
					}
					else
					{
						// check if its the default segment description	( M L Z N )
						if ( seqSegments1.getLength() == 4 )
						{
							if ( ( seqSegments1[ 0 ].Command == EnhancedCustomShapeSegmentCommand::MOVETO )
								&& ( seqSegments1[ 1 ].Command == EnhancedCustomShapeSegmentCommand::LINETO )
								&& ( seqSegments1[ 2 ].Command == EnhancedCustomShapeSegmentCommand::CLOSESUBPATH )
								&& ( seqSegments1[ 3 ].Command == EnhancedCustomShapeSegmentCommand::ENDSUBPATH ) )
								bIsDefaultGeometry = true;
						}
					}
				}
			}
			else if ( pDefCustomShape && ( ( pDefCustomShape->nElements == 0 ) || ( pDefCustomShape->pElements == 0 ) ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_STRETCHX :
		{
			const rtl::OUString	sStretchX( RTL_CONSTASCII_USTRINGPARAM ( "StretchX" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchX );
			if ( pAny && pDefCustomShape )
			{
				sal_Int32 nStretchX = 0;
				if ( *pAny >>= nStretchX )
				{
					if ( pDefCustomShape->nXRef == nStretchX )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( pDefCustomShape->nXRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_STRETCHY :
		{
			const rtl::OUString	sStretchY( RTL_CONSTASCII_USTRINGPARAM ( "StretchY" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sStretchY );
			if ( pAny && pDefCustomShape )
			{
				sal_Int32 nStretchY = 0;
				if ( *pAny >>= nStretchY )
				{
					if ( pDefCustomShape->nYRef == nStretchY )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( pDefCustomShape->nYRef == DEFAULT_MINIMUM_SIGNED_COMPARE ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_EQUATIONS :
		{
			const rtl::OUString	sEquations( RTL_CONSTASCII_USTRINGPARAM( "Equations" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sEquations );
			if ( pAny && pDefCustomShape && pDefCustomShape->nCalculation && pDefCustomShape->pCalculation )
			{
				com::sun::star::uno::Sequence< rtl::OUString > seqEquations1, seqEquations2;
				if ( *pAny >>= seqEquations1 )
				{
					sal_Int32 i, nCount = pDefCustomShape->nCalculation;
					seqEquations2.realloc( nCount );

					const SvxMSDffCalculationData* pData = pDefCustomShape->pCalculation;
					for ( i = 0; i < nCount; i++, pData++ )
						seqEquations2[ i ] = EnhancedCustomShape2d::GetEquation( pData->nFlags, pData->nVal[ 0 ], pData->nVal[ 1 ], pData->nVal[ 2 ] );

					if ( seqEquations1 == seqEquations2 )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( ( pDefCustomShape->nCalculation == 0 ) || ( pDefCustomShape->pCalculation == 0 ) ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_TEXTFRAMES :
		{
			const rtl::OUString	sTextFrames( RTL_CONSTASCII_USTRINGPARAM( "TextFrames" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sPath, sTextFrames );
			if ( pAny && pDefCustomShape && pDefCustomShape->nTextRect && pDefCustomShape->pTextRect )
			{
				com::sun::star::uno::Sequence< com::sun::star::drawing::EnhancedCustomShapeTextFrame > seqTextFrames1, seqTextFrames2;
				if ( *pAny >>= seqTextFrames1 )
				{
					sal_Int32 i, nCount = pDefCustomShape->nTextRect;
					seqTextFrames2.realloc( nCount );
					const SvxMSDffTextRectangles* pRectangles = pDefCustomShape->pTextRect;
					for ( i = 0; i < nCount; i++, pRectangles++ )
					{
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].TopLeft.First,	  pRectangles->nPairA.nValA );
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].TopLeft.Second,	  pRectangles->nPairA.nValB );
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].BottomRight.First,  pRectangles->nPairB.nValA );
						EnhancedCustomShape2d::SetEnhancedCustomShapeParameter( seqTextFrames2[ i ].BottomRight.Second, pRectangles->nPairB.nValB );
					}
					if ( seqTextFrames1 == seqTextFrames2 )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( ( pDefCustomShape->nTextRect == 0 ) || ( pDefCustomShape->pTextRect == 0 ) ) )
				bIsDefaultGeometry = true;
		}
		break;

		case DEFAULT_HANDLES :
		{
			const rtl::OUString	sHandles( RTL_CONSTASCII_USTRINGPARAM( "Handles" ) );
			pAny = ((SdrCustomShapeGeometryItem&)aGeometryItem).GetPropertyValueByName( sHandles );
			if ( pAny && pDefCustomShape && pDefCustomShape->nHandles && pDefCustomShape->pHandles )
			{
				com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValues > seqHandles1, seqHandles2;
				if ( *pAny >>= seqHandles1 )
				{
					sal_Int32 i, n, nCount = pDefCustomShape->nHandles;
					const SvxMSDffHandle* pData = pDefCustomShape->pHandles;
					seqHandles2.realloc( nCount );
					for ( i = 0; i < nCount; i++, pData++ )
					{
						sal_Int32 nPropertiesNeeded = 1;	// position is always needed
						sal_Int32 nFlags = pData->nFlags;
						if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
							nPropertiesNeeded++;
						if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
							nPropertiesNeeded++;
						if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
							nPropertiesNeeded++;
						if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
						{
							nPropertiesNeeded++;
							if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
							{
								if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
									nPropertiesNeeded++;
								if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
									nPropertiesNeeded++;
							}
						}
						else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
						{
							if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
								nPropertiesNeeded++;
							if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
								nPropertiesNeeded++;
							if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
								nPropertiesNeeded++;
							if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
								nPropertiesNeeded++;
						}

						n = 0;
						com::sun::star::beans::PropertyValues& rPropValues = seqHandles2[ i ];
						rPropValues.realloc( nPropertiesNeeded );

						// POSITION
						{
							const rtl::OUString	sPosition( RTL_CONSTASCII_USTRINGPARAM ( "Position" ) );
							::com::sun::star::drawing::EnhancedCustomShapeParameterPair aPosition;
							EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.First, pData->nPositionX, true, true );
							EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aPosition.Second, pData->nPositionY, true, false );
							rPropValues[ n ].Name = sPosition;
							rPropValues[ n++ ].Value <<= aPosition;
						}
						if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_X )
						{
							const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
							bool bMirroredX = true;
							rPropValues[ n ].Name = sMirroredX;
							rPropValues[ n++ ].Value <<= bMirroredX;
						}
						if ( nFlags & MSDFF_HANDLE_FLAGS_MIRRORED_Y )
						{
							const rtl::OUString	sMirroredY( RTL_CONSTASCII_USTRINGPARAM ( "MirroredY" ) );
							bool bMirroredY = true;
							rPropValues[ n ].Name = sMirroredY;
							rPropValues[ n++ ].Value <<= bMirroredY;
						}
						if ( nFlags & MSDFF_HANDLE_FLAGS_SWITCHED )
						{
							const rtl::OUString	sSwitched( RTL_CONSTASCII_USTRINGPARAM ( "Switched" ) );
							bool bSwitched = true;
							rPropValues[ n ].Name = sSwitched;
							rPropValues[ n++ ].Value <<= bSwitched;
						}
						if ( nFlags & MSDFF_HANDLE_FLAGS_POLAR )
						{
							const rtl::OUString	sPolar( RTL_CONSTASCII_USTRINGPARAM ( "Polar" ) );
							::com::sun::star::drawing::EnhancedCustomShapeParameterPair aCenter;
							EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.First,  pData->nCenterX,
								( nFlags & MSDFF_HANDLE_FLAGS_CENTER_X_IS_SPECIAL ) != 0, true  );
							EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aCenter.Second, pData->nCenterY,
								( nFlags & MSDFF_HANDLE_FLAGS_CENTER_Y_IS_SPECIAL ) != 0, false );
							rPropValues[ n ].Name = sPolar;
							rPropValues[ n++ ].Value <<= aCenter;
							if ( nFlags & MSDFF_HANDLE_FLAGS_RADIUS_RANGE )
							{
								if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
								{
									const rtl::OUString	sRadiusRangeMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMinimum" ) );
									::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMinimum;
									EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMinimum, pData->nRangeXMin,
										( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
									rPropValues[ n ].Name = sRadiusRangeMinimum;
									rPropValues[ n++ ].Value <<= aRadiusRangeMinimum;
								}
								if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
								{
									const rtl::OUString	sRadiusRangeMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RadiusRangeMaximum" ) );
									::com::sun::star::drawing::EnhancedCustomShapeParameter aRadiusRangeMaximum;
									EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRadiusRangeMaximum, pData->nRangeXMax,
										( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
									rPropValues[ n ].Name = sRadiusRangeMaximum;
									rPropValues[ n++ ].Value <<= aRadiusRangeMaximum;
								}
							}
						}
						else if ( nFlags & MSDFF_HANDLE_FLAGS_RANGE )
						{
							if ( pData->nRangeXMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
							{
								const rtl::OUString	sRangeXMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMinimum" ) );
								::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMinimum;
								EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMinimum, pData->nRangeXMin,
									( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MIN_IS_SPECIAL ) != 0, true  );
								rPropValues[ n ].Name = sRangeXMinimum;
								rPropValues[ n++ ].Value <<= aRangeXMinimum;
							}
							if ( pData->nRangeXMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
							{
								const rtl::OUString	sRangeXMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeXMaximum" ) );
								::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeXMaximum;
								EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeXMaximum, pData->nRangeXMax,
									( nFlags & MSDFF_HANDLE_FLAGS_RANGE_X_MAX_IS_SPECIAL ) != 0, false );
								rPropValues[ n ].Name = sRangeXMaximum;
								rPropValues[ n++ ].Value <<= aRangeXMaximum;
							}
							if ( pData->nRangeYMin != DEFAULT_MINIMUM_SIGNED_COMPARE )
							{
								const rtl::OUString	sRangeYMinimum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMinimum" ) );
								::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMinimum;
								EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMinimum, pData->nRangeYMin,
									( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MIN_IS_SPECIAL ) != 0, true );
								rPropValues[ n ].Name = sRangeYMinimum;
								rPropValues[ n++ ].Value <<= aRangeYMinimum;
							}
							if ( pData->nRangeYMax != DEFAULT_MAXIMUM_SIGNED_COMPARE )
							{
								const rtl::OUString	sRangeYMaximum( RTL_CONSTASCII_USTRINGPARAM ( "RangeYMaximum" ) );
								::com::sun::star::drawing::EnhancedCustomShapeParameter aRangeYMaximum;
								EnhancedCustomShape2d::SetEnhancedCustomShapeHandleParameter( aRangeYMaximum, pData->nRangeYMax,
									( nFlags & MSDFF_HANDLE_FLAGS_RANGE_Y_MAX_IS_SPECIAL ) != 0, false );
								rPropValues[ n ].Name = sRangeYMaximum;
								rPropValues[ n++ ].Value <<= aRangeYMaximum;
							}
						}
					}
					if ( seqHandles1 == seqHandles2 )
						bIsDefaultGeometry = true;
				}
			}
			else if ( pDefCustomShape && ( ( pDefCustomShape->nHandles == 0 ) || ( pDefCustomShape->pHandles == 0 ) ) )
				bIsDefaultGeometry = true;
		}
		break;
	}
	return bIsDefaultGeometry;
}

void SdrObjCustomShape::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
	rInfo.bResizeFreeAllowed=fObjectRotation == 0.0;
	rInfo.bResizePropAllowed=true;
	rInfo.bRotateFreeAllowed=true;
	rInfo.bRotate90Allowed  =true;
	rInfo.bMirrorFreeAllowed=true;
	rInfo.bMirror45Allowed  =true;
	rInfo.mbMirror90Allowed  =true;
	rInfo.mbTransparenceAllowed = false;
	rInfo.mbGradientAllowed = false;
	rInfo.mbShearAllowed     =true;
	rInfo.mbEdgeRadiusAllowed=false;
	rInfo.bNoContortion     =true;

	// #i37011#
	if ( mXRenderedCustomShape.is() )
	{
		const SdrObject* pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
		if ( pRenderedCustomShape )
		{
			// #i37262#
			// Iterate self over the contained objects, since there are combinations of
			// polygon and curve objects. In that case, aInfo.mbCanConvToPath and
			// aInfo.mbCanConvToPoly would be false. What is needed here is an or, not an and.
			SdrObjListIter aIterator(*pRenderedCustomShape);
			while(aIterator.IsMore())
			{
				SdrObject* pCandidate = aIterator.Next();
				SdrObjTransformInfoRec aInfo;
				pCandidate->TakeObjInfo(aInfo);

				// set path and poly conversion if one is possible since
				// this object will first be broken
				const bool bCanConvToPathOrPoly(aInfo.mbCanConvToPath || aInfo.mbCanConvToPoly);
				
                if(rInfo.mbCanConvToPath != bCanConvToPathOrPoly)
				{
					rInfo.mbCanConvToPath = bCanConvToPathOrPoly;
				}

				if(rInfo.mbCanConvToPoly != bCanConvToPathOrPoly)
				{
					rInfo.mbCanConvToPoly = bCanConvToPathOrPoly;
				}

				if(rInfo.mbCanConvToContour != aInfo.mbCanConvToContour)
				{
					rInfo.mbCanConvToContour = aInfo.mbCanConvToContour;
				}
			}
		}
	}
}

sal_uInt16 SdrObjCustomShape::GetObjIdentifier() const
{
	return sal_uInt16(OBJ_CUSTOMSHAPE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
	// call parent
	SdrTextObj::setSdrObjectTransformation(rTransformation);

	// TTTT: extract mirror flags and trigger SetMirroredX/SetMirroredY
	//		const rtl::OUString	sMirroredX( RTL_CONSTASCII_USTRINGPARAM ( "MirroredX" ) );
	//		PropertyValue aPropVal;
	//		aPropVal.Name = sMirroredX;
	//		aPropVal.Value <<= bHorz;
	//		aGeometryItem.SetPropertyValue( aPropVal );

	// TTTT: extra-actions from old NbcSetSnapRect

	// TTTT: extra evtl. adapt InteractionHandles
	//std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
	//while ( aIter != aInteractionHandles.end() )
	//{
	//	try
	//	{
	//		if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
	//			aIter->xInteraction->setControllerPosition( aIter->aPosition );
	//		if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_X )
	//		{
	//			sal_Int32 nX = ( aIter->aPosition.X - aOld.Left() ) + aRect.Left();
	//			aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( nX, aIter->xInteraction->getPosition().Y ) );
	//		}
	//		if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_ABSOLUTE_Y )
	//		{
	//			sal_Int32 nY = ( aIter->aPosition.Y - aOld.Top() ) + aRect.Top();
	//			aIter->xInteraction->setControllerPosition( com::sun::star::awt::Point( aIter->xInteraction->getPosition().X, nY ) );
	//		}
	//	}
	//	catch ( const uno::RuntimeException& )
	//	{
	//	}
	//	aIter++;
	//}

	// TTTT: fObjectRotation?

}

////////////////////////////////////////////////////////////////////////////////////////////////////

// #i38892#
void SdrObjCustomShape::ImpCheckCustomGluePointsAreAdded()
{
	const SdrObject* pSdrObject = GetSdrObjectFromCustomShape();

	if(pSdrObject)
	{
		const SdrGluePointList* pSource = pSdrObject->GetGluePointList();

		if(pSource && pSource->GetCount())
		{
			if(!SdrTextObj::GetGluePointList())
			{
				SdrTextObj::ForceGluePointList();
			}

			const SdrGluePointList* pList = SdrTextObj::GetGluePointList();

			if(pList)
			{
				SdrGluePointList aNewList;
				sal_uInt16 a;

				// build transform matrix from helper object to local
				basegfx::B2DHomMatrix aTransFromHelperToLocal(pSdrObject->getSdrObjectTransformation());
				aTransFromHelperToLocal.invert();
				aTransFromHelperToLocal = getSdrObjectTransformation() * aTransFromHelperToLocal;

				for(a = 0; a < pSource->GetCount(); a++)
				{
					SdrGluePoint aCopy((*pSource)[a]);
					aCopy.SetUserDefined(false);

					basegfx::B2DPoint aGluePos(aCopy.GetPos());
					aGluePos *= aTransFromHelperToLocal;
					aCopy.SetPos(aGluePos);

					aNewList.Insert(aCopy);
				}

				for(a = 0; a < pList->GetCount(); a++)
				{
					const SdrGluePoint& rCandidate = (*pList)[a];

					if(rCandidate.IsUserDefined())
					{
						aNewList.Insert(rCandidate);
					}
				}

				// copy new list to local. This is NOT very convenient behaviour, the local
				// GluePointList should not be set, but be delivered by using GetGluePointList(),
				// maybe on demand. Since the local object is changed here, this is assumed to
				// be a result of GetGluePointList and thus the list is copied
				if(mpPlusData)
				{
					*mpPlusData->mpGluePoints = aNewList;
				}
			}
		}
	}
}

// #i38892#
const SdrGluePointList* SdrObjCustomShape::GetGluePointList() const
{
	((SdrObjCustomShape*)this)->ImpCheckCustomGluePointsAreAdded();
	return SdrTextObj::GetGluePointList();
}

// #i38892#
SdrGluePointList* SdrObjCustomShape::ForceGluePointList()
{
	if(SdrTextObj::ForceGluePointList())
	{
		ImpCheckCustomGluePointsAreAdded();
		return SdrTextObj::ForceGluePointList();
	}
	else
	{
		return 0L;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::AddToHdlList(SdrHdlList& rHdlList) const
{
	// add handles from parent object
	SdrTextObj::AddToHdlList(rHdlList);

	// add own handles
	std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

	for(sal_uInt32 a(0); a < aInteractionHandles.size(); a++)
	{
		if(aInteractionHandles[a].xInteraction.is())
		{
			try
			{
				const com::sun::star::awt::Point aPosition(aInteractionHandles[a].xInteraction->getPosition());
				SdrHdl* pHdl = new SdrHdl(rHdlList, this, HDL_CUSTOMSHAPE1, basegfx::B2DPoint(aPosition.X, aPosition.Y));
				pHdl->SetPointNum(a);
			}
			catch ( const uno::RuntimeException& )
			{
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrObjCustomShape::hasSpecialDrag() const
{
	return true;
}

bool SdrObjCustomShape::beginSpecialDrag(SdrDragStat& rDrag) const
{
	const SdrHdl* pHdl = rDrag.GetActiveHdl();

    if(pHdl && HDL_CUSTOMSHAPE1 == pHdl->GetKind())
	{
		rDrag.SetEndDragChangesAttributes(true);
		rDrag.SetNoSnap(true);
	}
	else
	{
		const SdrHdl* pHdl2 = rDrag.GetActiveHdl();
		const SdrHdlKind eHdl((pHdl2 == NULL) ? HDL_MOVE : pHdl2->GetKind());

        switch( eHdl )
		{
			case HDL_UPLFT :
			case HDL_UPPER :
			case HDL_UPRGT :
			case HDL_LEFT  :
			case HDL_RIGHT :
			case HDL_LWLFT :
			case HDL_LOWER :
			case HDL_LWRGT :
			case HDL_MOVE  :
			{
    			break;
			}
			default:
            {
				return false;
            }
		}
	}

    return true;
}

void SdrObjCustomShape::DragMoveCustomShapeHdl( const basegfx::B2DPoint& rDestination, const sal_uInt32 nCustomShapeHdlNum, SdrObjCustomShape* pObj ) const
{
	std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( pObj ) );
	if ( nCustomShapeHdlNum < aInteractionHandles.size() )
	{
		SdrCustomShapeInteraction aInteractionHandle( aInteractionHandles[ nCustomShapeHdlNum ] );
		if ( aInteractionHandle.xInteraction.is() )
		{
			try
			{
				com::sun::star::awt::Point aPt( basegfx::fround(rDestination.getX()), basegfx::fround(rDestination.getY()) );
				if ( aInteractionHandle.nMode & CUSTOMSHAPE_HANDLE_MOVE_SHAPE )
				{
					const sal_Int32 nXDiff(aPt.X - aInteractionHandle.aPosition.X);
					const sal_Int32 nYDiff(aPt.Y - aInteractionHandle.aPosition.Y);

					sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(nXDiff, nYDiff));

					std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
					while ( aIter != aInteractionHandles.end() )
					{
						if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
						{
							if ( aIter->xInteraction.is() )
								aIter->xInteraction->setControllerPosition( aIter->aPosition );
						}
						aIter++;
					}
				}
				aInteractionHandle.xInteraction->setControllerPosition( aPt );
			}
			catch ( const uno::RuntimeException& )
			{
			}
		}
	}
}

bool SdrObjCustomShape::applySpecialDrag(SdrDragStat& rDrag)
{
	const SdrHdl* pHdl = rDrag.GetActiveHdl();
	const SdrHdlKind eHdl((pHdl == NULL) ? HDL_MOVE : pHdl->GetKind());
	bool bRetval(true);

    switch(eHdl)
	{
		case HDL_CUSTOMSHAPE1 :
		{
			rDrag.SetEndDragChangesGeoAndAttributes(true);
		    DragMoveCustomShapeHdl( rDrag.GetNow(), pHdl->GetPointNum(), this );
		    InvalidateRenderGeometry();
		    SetChanged();
            break;
		}

		case HDL_UPLFT :
		case HDL_UPPER :
		case HDL_UPRGT :
		case HDL_LEFT  :
		case HDL_RIGHT :
		case HDL_LWLFT :
		case HDL_LOWER :
		case HDL_LWRGT :
		{
			bRetval = SdrTextObj::applySpecialDrag(rDrag);
			break;
		}
		case HDL_MOVE :
		{
			sdr::legacy::MoveSdrObject(*this, Size(rDrag.GetDX(), rDrag.GetDY()));
			break;
		}
		default: break;
	}

    return bRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrObjCustomShape::DragCreateObject( SdrDragStat& rStat )
{
	basegfx::B2DRange aRange(rStat.TakeCreateRange());

	std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );

	sal_uInt32 nDefaultObjectSizeWidth = 3000;		// default width from SDOptions ?
	sal_uInt32 nDefaultObjectSizeHeight= 3000;

	if ( ImpVerticalSwitch( *this ) )
	{
		SetMirroredX( aRange.getMinX() > aRange.getMaxX() );

		aRange = basegfx::B2DRange( rStat.GetNow(), basegfx::B2DTuple( nDefaultObjectSizeWidth, nDefaultObjectSizeHeight ) );
		// subtracting the horizontal difference of the latest handle from shape position
		if ( aInteractionHandles.size() )
		{
			sal_Int32 nHandlePos = aInteractionHandles[ aInteractionHandles.size() - 1 ].xInteraction->getPosition().X;
			const basegfx::B2DRange aOldObjRange(sdr::legacy::GetLogicRange(*this));
		
			aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(aOldObjRange.getMinX() - nHandlePos, 0.0));
		}
	}

	rStat.SetActionRange( aRange );
	sdr::legacy::SetLogicRange(*this, aRange);

	std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );
	while ( aIter != aInteractionHandles.end() )
	{
		try
		{
			if ( aIter->nMode & CUSTOMSHAPE_HANDLE_CREATE_FIXED )
			{
				aIter->xInteraction->setControllerPosition( 
					awt::Point( basegfx::fround(rStat.GetStart().getX()), basegfx::fround(rStat.GetStart().getY()) ) );
			}
		}
		catch ( const uno::RuntimeException& )
		{
		}
		aIter++;
	}

	ActionChanged();
}

bool SdrObjCustomShape::MovCreate(SdrDragStat& rStat)
{
	SdrView& rView = rStat.GetSdrViewFromSdrDragStat();		// #i37448#
	if( rView.IsSolidDragging() )
	{
		InvalidateRenderGeometry();
	}
	DragCreateObject( rStat );
	ActionChanged();
	
	return true;
}

bool SdrObjCustomShape::EndCreate( SdrDragStat& rStat, SdrCreateCmd eCmd )
{
	DragCreateObject( rStat );

	if ( bTextFrame )
	{
		const Rectangle aOldObjRect(sdr::legacy::GetLogicRect(*this));

		if ( IsAutoGrowHeight() )
		{
			// MinTextHeight
			long nHgt=aOldObjRect.GetHeight()-1;
			if (nHgt==1) nHgt=0;
			SetMinTextFrameHeight( nHgt );
		}
		if ( IsAutoGrowWidth() )
		{
			// MinTextWidth
			long nWdt=aOldObjRect.GetWidth()-1;
			if (nWdt==1) nWdt=0;
			SetMinTextFrameWidth( nWdt );
		}
		
		// Textrahmen neu berechnen
		AdjustTextFrameWidthAndHeight();
	}
	ActionChanged();
	return ( eCmd == SDRCREATE_FORCEEND || rStat.GetPointAnz() >= 2 );
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeCreatePoly(const SdrDragStat& /*rDrag*/) const
{
	return GetLineGeometry( this, false );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// in context with the SdrObjCustomShape the TextAutoGrowHeight == true -> Resize Shape to fit text,
//									   the TextAutoGrowWidth  == true -> Word wrap text in Shape
bool SdrObjCustomShape::IsAutoGrowHeight() const
{
	const SfxItemSet& rSet = GetMergedItemSet();
	bool bIsAutoGrowHeight = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
	if ( bIsAutoGrowHeight && IsVerticalWriting() )
		bIsAutoGrowHeight = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == false;
	return bIsAutoGrowHeight;
}
bool SdrObjCustomShape::IsAutoGrowWidth() const
{
	const SfxItemSet& rSet = GetMergedItemSet();
	bool bIsAutoGrowWidth = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();
	if ( bIsAutoGrowWidth && !IsVerticalWriting() )
		bIsAutoGrowWidth = ((SdrOnOffItem&)(rSet.Get(SDRATTR_TEXT_WORDWRAP))).GetValue() == false;
	return bIsAutoGrowWidth;
}

/* The following method is identical to the SdrTextObj::SetVerticalWriting method, the only difference
   is that the SdrAutoGrowWidthItem and SdrAutoGrowHeightItem are not exchanged if the vertical writing
   mode has been changed */

void SdrObjCustomShape::SetVerticalWriting( bool bVertical )
{
	ForceOutlinerParaObject();

	OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();

	DBG_ASSERT( pOutlinerParaObject, "SdrTextObj::SetVerticalWriting() without OutlinerParaObject!" );

	if( pOutlinerParaObject )
	{
		if(pOutlinerParaObject->IsVertical() != (bool)bVertical)
		{
			// get item settings
			const SfxItemSet& rSet = GetObjectItemSet();

			// #103516# Also exchange hor/ver adjust items
			SdrTextHorzAdjust eHorz = ((SdrTextHorzAdjustItem&)(rSet.Get(SDRATTR_TEXT_HORZADJUST))).GetValue();
			SdrTextVertAdjust eVert = ((SdrTextVertAdjustItem&)(rSet.Get(SDRATTR_TEXT_VERTADJUST))).GetValue();

			// rescue object size
			Rectangle aObjectRect(sdr::legacy::GetSnapRect(*this));

			// prepare ItemSet to set exchanged width and height items
			SfxItemSet aNewSet(*rSet.GetPool(),
				SDRATTR_TEXT_AUTOGROWHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT,
				// #103516# Expanded item ranges to also support hor and ver adjust.
				SDRATTR_TEXT_VERTADJUST, SDRATTR_TEXT_VERTADJUST,
				SDRATTR_TEXT_AUTOGROWWIDTH, SDRATTR_TEXT_HORZADJUST,
				0, 0);

			aNewSet.Put(rSet);

			// #103516# Exchange horz and vert adjusts
			switch(eVert)
			{
				case SDRTEXTVERTADJUST_TOP: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT)); break;
				case SDRTEXTVERTADJUST_CENTER: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER)); break;
				case SDRTEXTVERTADJUST_BOTTOM: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT)); break;
				case SDRTEXTVERTADJUST_BLOCK: aNewSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK)); break;
			}
			switch(eHorz)
			{
				case SDRTEXTHORZADJUST_LEFT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BOTTOM)); break;
				case SDRTEXTHORZADJUST_CENTER: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER)); break;
				case SDRTEXTHORZADJUST_RIGHT: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP)); break;
				case SDRTEXTHORZADJUST_BLOCK: aNewSet.Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_BLOCK)); break;
			}

			SetObjectItemSet( aNewSet );
			pOutlinerParaObject = GetOutlinerParaObject();
			if ( pOutlinerParaObject )
				pOutlinerParaObject->SetVertical(bVertical);
	
			// restore object size
			sdr::legacy::SetSnapRect(*this, aObjectRect);
		}
	}
}
bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight(Rectangle& rR, bool bHgt, bool bWdt) const
{
 	if ( HasText() && !rR.IsEmpty() )
	{
		bool bWdtGrow=bWdt && IsAutoGrowWidth();
		bool bHgtGrow=bHgt && IsAutoGrowHeight();
		if ( bWdtGrow || bHgtGrow )
		{
			Rectangle aR0(rR);
			long nHgt=0,nMinHgt=0,nMaxHgt=0;
			long nWdt=0,nMinWdt=0,nMaxWdt=0;
			Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
			Size aMaxSiz(100000,100000);

			if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getX())) 
			{
				aMaxSiz.Width() = basegfx::fround(getSdrModelFromSdrObject().GetMaxObjectScale().getX());
			}

			if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY())) 
			{
				aMaxSiz.Height() = basegfx::fround(getSdrModelFromSdrObject().GetMaxObjectScale().getY());
			}

			if (bWdtGrow)
			{
				nMinWdt=GetMinTextFrameWidth();
				nMaxWdt=GetMaxTextFrameWidth();
				if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
				if (nMinWdt<=0) nMinWdt=1;
				aSiz.Width()=nMaxWdt;
			}
			if (bHgtGrow)
			{
				nMinHgt=GetMinTextFrameHeight();
				nMaxHgt=GetMaxTextFrameHeight();
				if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
				if (nMinHgt<=0) nMinHgt=1;
				aSiz.Height()=nMaxHgt;
			}
			long nHDist=GetTextLeftDistance()+GetTextRightDistance();
			long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
			aSiz.Width()-=nHDist;
			aSiz.Height()-=nVDist;
			if ( aSiz.Width() < 2 )
				aSiz.Width() = 2;   // Mindestgroesse 2
			if ( aSiz.Height() < 2 )
				aSiz.Height() = 2; // Mindestgroesse 2

			if(pEdtOutl)
			{
				pEdtOutl->SetMaxAutoPaperSize( aSiz );
				if (bWdtGrow)
				{
					Size aSiz2(pEdtOutl->CalcTextSize());
					nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
					if (bHgtGrow) nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
				} else
				{
					nHgt=pEdtOutl->GetTextHeight()+1; // lieber etwas Tolleranz
				}
			}
			else
			{
				Outliner& rOutliner=ImpGetDrawOutliner();
				rOutliner.SetPaperSize(aSiz);
				rOutliner.SetUpdateMode(true);
				// !!! hier sollte ich wohl auch noch mal die Optimierung mit
				// bPortionInfoChecked usw einbauen
				OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
				if( pOutlinerParaObject != NULL )
				{
					rOutliner.SetText(*pOutlinerParaObject);
					rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
				}
				if ( bWdtGrow )
				{
					Size aSiz2(rOutliner.CalcTextSize());
					nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
					if ( bHgtGrow )
						nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
				}
				else
					nHgt = rOutliner.GetTextHeight()+1; // lieber etwas Tolleranz
				rOutliner.Clear();
			}
			if ( nWdt < nMinWdt )
				nWdt = nMinWdt;
			if ( nWdt > nMaxWdt )
				nWdt = nMaxWdt;
			nWdt += nHDist;
			if ( nWdt < 1 )
				nWdt = 1; // nHDist kann auch negativ sein
			if ( nHgt < nMinHgt )
				nHgt = nMinHgt;
			if ( nHgt > nMaxHgt )
				nHgt = nMaxHgt;
			nHgt+=nVDist;
			if ( nHgt < 1 )
				nHgt = 1; // nVDist kann auch negativ sein
			long nWdtGrow = nWdt-(rR.Right()-rR.Left());
			long nHgtGrow = nHgt-(rR.Bottom()-rR.Top());
			if ( nWdtGrow == 0 )
				bWdtGrow = false;
			if ( nHgtGrow == 0 )
				bHgtGrow=false;
			if ( bWdtGrow || bHgtGrow )
			{
				if ( bWdtGrow )
				{
					SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
					if ( eHAdj == SDRTEXTHORZADJUST_LEFT )
						rR.Right()+=nWdtGrow;
					else if ( eHAdj == SDRTEXTHORZADJUST_RIGHT )
						rR.Left()-=nWdtGrow;
					else
					{
						long nWdtGrow2=nWdtGrow/2;
						rR.Left()-=nWdtGrow2;
						rR.Right()=rR.Left()+nWdt;
					}
				}
				if ( bHgtGrow )
				{
					SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
					if ( eVAdj == SDRTEXTVERTADJUST_TOP )
						rR.Bottom()+=nHgtGrow;
					else if ( eVAdj == SDRTEXTVERTADJUST_BOTTOM )
						rR.Top()-=nHgtGrow;
					else
					{
						long nHgtGrow2=nHgtGrow/2;
						rR.Top()-=nHgtGrow2;
						rR.Bottom()=rR.Top()+nHgt;
					}
				}
				const long aOldRotation(sdr::legacy::GetRotateAngle(*this));
				if ( aOldRotation )
				{
					Point aD1(rR.TopLeft());
					aD1-=aR0.TopLeft();
					Point aD2(aD1);
					RotatePoint(aD2,Point(),sin(aOldRotation*nPi180), cos(aOldRotation*nPi180));
					aD2-=aD1;
					rR.Move(aD2.X(),aD2.Y());
				}
				return true;
			}
		}
	}
	return false;
}

basegfx::B2DRange SdrObjCustomShape::ImpCalculateTextFrame(const bool bHgt, const bool bWdt)
{
	basegfx::B2DRange aReturnValue;
	const basegfx::B2DRange aOldObjRange(sdr::legacy::GetLogicRange(*this));

	// initial text rectangle
	const basegfx::B2DRange aOldTextRange(aOldObjRange);

	// new text rectangle returned from the custom shape renderer,
	// it depends to the current logical shape size
	const basegfx::B2DRange aNewTextRange(getRawUnifiedTextRange()); 

	// new text rectangle is being tested by AdjustTextFrameWidthAndHeight to ensure
	// that the new text rectangle is matching the current text size from the outliner
	basegfx::B2DRange aAdjustedTextRange(aNewTextRange);
	
	Rectangle aTemp( // TTTT: adapt AdjustTextFrameWidthAndHeight, remove here
		(sal_Int32)floor(aAdjustedTextRange.getMinX()), (sal_Int32)floor(aAdjustedTextRange.getMinY()),
		(sal_Int32)ceil(aAdjustedTextRange.getMaxX()), (sal_Int32)ceil(aAdjustedTextRange.getMaxY()));

	if(AdjustTextFrameWidthAndHeight(aTemp, bHgt, bWdt))	
	{
		// TTTT: adapt AdjustTextFrameWidthAndHeight, remove here
		aAdjustedTextRange = basegfx::B2DRange(aTemp.Left(), aTemp.Top(), aTemp.Right(), aTemp.Bottom());

		if((aAdjustedTextRange != aNewTextRange) && (aOldTextRange != aAdjustedTextRange))
		{
			const basegfx::B2DVector aScale(aOldTextRange.getRange() / aNewTextRange.getRange());
			const basegfx::B2DVector aTopLeftDiff((aAdjustedTextRange.getMinimum() - aNewTextRange.getMinimum()) * aScale);
			const basegfx::B2DVector aBottomRightDiff((aAdjustedTextRange.getMaximum() - aNewTextRange.getMaximum()) * aScale);

			aReturnValue = basegfx::B2DRange(
				aReturnValue.getMinimum() + aTopLeftDiff,
				aReturnValue.getMaximum() + aBottomRightDiff);
		}
	}

	return aReturnValue;
}

bool SdrObjCustomShape::AdjustTextFrameWidthAndHeight(bool bHgt, bool bWdt)
{
	const basegfx::B2DRange aNewTextRange(ImpCalculateTextFrame(bHgt, bWdt));
	const basegfx::B2DRange aOldObjRange(sdr::legacy::GetLogicRange(*this));
	bool bRet(!aNewTextRange.isEmpty() && (aNewTextRange != aOldObjRange));
	
	if(bRet)
	{
		// taking care of handles that should not been changed
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);
		std::vector< SdrCustomShapeInteraction > aInteractionHandles( GetInteractionHandles( this ) );
		sdr::legacy::SetLogicRange(*this, aNewTextRange);
		std::vector< SdrCustomShapeInteraction >::iterator aIter( aInteractionHandles.begin() );

		while ( aIter != aInteractionHandles.end() )
		{
			try
			{
				if ( aIter->nMode & CUSTOMSHAPE_HANDLE_RESIZE_FIXED )
					aIter->xInteraction->setControllerPosition( aIter->aPosition );
			}
			catch ( const uno::RuntimeException& )
			{
			}

			aIter++;
		}

		InvalidateRenderGeometry();
		SetChanged();
	}

	return bRet;
}

bool SdrObjCustomShape::BegTextEdit( SdrOutliner& rOutl )
{
	return SdrTextObj::BegTextEdit( rOutl );
}

void SdrObjCustomShape::TakeTextEditArea(basegfx::B2DVector* pPaperMin, basegfx::B2DVector* pPaperMax, basegfx::B2DRange* pViewInit, basegfx::B2DRange* pViewMin) const
{
	// get TextRange without shear, rotate and mirror, just scaled
	// and centered in logic coordinates
	basegfx::B2DRange aViewInit(getScaledCenteredTextRange(*this));

	basegfx::B2DVector aPaperMin;
	basegfx::B2DVector aPaperMax;
	basegfx::B2DVector aAnkSiz(aViewInit.getRange());
	basegfx::B2DVector aMaxSiz(1000000.0, 1000000.0);

	if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getX())) 
	{
		aMaxSiz.setX(getSdrModelFromSdrObject().GetMaxObjectScale().getX());
	}

	if(!basegfx::fTools::equalZero(getSdrModelFromSdrObject().GetMaxObjectScale().getY())) 
	{
		aMaxSiz.setY(getSdrModelFromSdrObject().GetMaxObjectScale().getY());
	}
	
	SdrTextHorzAdjust eHAdj(GetTextHorizontalAdjust());
	SdrTextVertAdjust eVAdj(GetTextVerticalAdjust());

	double fMinWdt(std::max(1.0, (double)GetMinTextFrameWidth()));
	double fMinHgt(std::max(1.0, (double)GetMinTextFrameHeight()));
	double fMaxWdt(GetMaxTextFrameWidth());
	double fMaxHgt(GetMaxTextFrameHeight());

	if(basegfx::fTools::equalZero(fMaxWdt) || basegfx::fTools::more(fMaxWdt, aMaxSiz.getX()))
	{
		fMaxWdt = aMaxSiz.getX();
	}

	if(basegfx::fTools::equalZero(fMaxHgt) || basegfx::fTools::more(fMaxHgt, aMaxSiz.getY()))
	{
		fMaxHgt = aMaxSiz.getY();
	}

	if(((SdrOnOffItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
	{
		if(IsVerticalWriting())
		{
			fMaxHgt = aAnkSiz.getY();
			fMinHgt = fMaxHgt;
		}
		else
		{
			fMaxWdt = aAnkSiz.getX();
			fMinWdt = fMaxWdt;
		}
	}

	aPaperMax.setX(fMaxWdt);
	aPaperMax.setY(fMaxHgt);

	aPaperMin.setX(fMinWdt);
	aPaperMin.setY(fMinHgt);

	if(pViewMin)
	{
		*pViewMin = aViewInit;
		const double fXFree(aAnkSiz.getX() - aPaperMin.getX());

		if(SDRTEXTHORZADJUST_LEFT == eHAdj)
		{
			*pViewMin = basegfx::B2DRange(
				pViewMin->getMinX(),
				pViewMin->getMinY(),
				pViewMin->getMaxX() - fXFree,
				pViewMin->getMaxY());
		}
		else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
		{
			*pViewMin = basegfx::B2DRange(
				pViewMin->getMinX() + fXFree,
				pViewMin->getMinY(),
				pViewMin->getMaxX(),
				pViewMin->getMaxY());
		}
		else 
		{ 
			const double fNewMinX(pViewMin->getMinX() + (fXFree * 0.5));
			*pViewMin = basegfx::B2DRange(
				fNewMinX,
				pViewMin->getMinY(),
				fNewMinX + aPaperMin.getX(),
				pViewMin->getMaxY());
		}

		const double fYFree(aAnkSiz.getY() - aPaperMin.getY());

		if(SDRTEXTVERTADJUST_TOP == eVAdj)
		{
			*pViewMin = basegfx::B2DRange(
				pViewMin->getMinX(),
				pViewMin->getMinY(),
				pViewMin->getMaxX(),
				pViewMin->getMaxY() - fYFree);
		}
		else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
		{
			*pViewMin = basegfx::B2DRange(
				pViewMin->getMinX(),
				pViewMin->getMinY() + fYFree,
				pViewMin->getMaxX(),
				pViewMin->getMaxY());
		}
		else 
		{ 
			const double fNewMinY(pViewMin->getMinY() + (fYFree * 0.5));
			*pViewMin = basegfx::B2DRange(
				pViewMin->getMinX(),
				fNewMinY,
				pViewMin->getMaxX(),
				fNewMinY + aPaperMin.getY());
		}
	}

	if(IsVerticalWriting())
	{
		aPaperMin.setX(0.0);
	}
	else
	{
		// #33102#
		aPaperMin.setY(0.0);
	}

	if( eHAdj != SDRTEXTHORZADJUST_BLOCK )
	{
		aPaperMin.setX(0.0);
	}

	// #103516# For complete ver adjust support, set paper min height to 0, here.
	if(SDRTEXTVERTADJUST_BLOCK != eVAdj )
	{
		aPaperMin.setY(0.0);
	}

	if(pPaperMin) 
	{
		*pPaperMin = aPaperMin;
	}

	if(pPaperMax) 
	{
		*pPaperMax = aPaperMax;
	}

	if(pViewInit)
	{
		*pViewInit = aViewInit;
	}
}
void SdrObjCustomShape::EndTextEdit( SdrOutliner& rOutl )
{
	SdrTextObj::EndTextEdit( rOutl );
	InvalidateRenderGeometry();
}
basegfx::B2DRange SdrObjCustomShape::getRawUnifiedTextRange() const
{
	// a candidate for being cached
	Reference< XCustomShapeEngine > xCustomShapeEngine(GetCustomShapeEngine(this));

	if(xCustomShapeEngine.is())
	{
		awt::Rectangle aR(xCustomShapeEngine->getTextBounds());

		if(aR.Width || aR.Height)
		{
			// the text bounds from CustomShapeEngine can be seen as scaled and translated
			// whereby scale is absolute (without mirrorings AFAIK). To get the unified range,
			// multiply with the inverse of M = T(obj) * S(obj). This can be done directly
			// here
			const double fX(aR.X - getSdrObjectTranslate().getX());
			const double fY(aR.Y - getSdrObjectTranslate().getY());
			const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
			const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));
			const double fW(aR.Width * fAbsInvScaleX);
			const double fH(aR.Height * fAbsInvScaleY);

			return basegfx::B2DRange(fX, fY, fX + fW, fY + fH);
		}
	}

	// per default the text range is the whole object range
	return basegfx::B2DRange::getUnitB2DRange();
}
basegfx::B2DRange SdrObjCustomShape::getUnifiedTextRange() const
{
	basegfx::B2DRange aRawRange(getRawUnifiedTextRange());
	const double fAbsInvScaleX(basegfx::fTools::equalZero(getSdrObjectScale().getX()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getX()));
	const double fAbsInvScaleY(basegfx::fTools::equalZero(getSdrObjectScale().getY()) ? 1.0 : 1.0 / fabs(getSdrObjectScale().getY()));

	// add/remove the text distances
	return basegfx::B2DRange(
		aRawRange.getMinX() + (GetTextLeftDistance() * fAbsInvScaleX),
		aRawRange.getMinY() + (GetTextUpperDistance() * fAbsInvScaleY),
		aRawRange.getMaxX() - (GetTextRightDistance() * fAbsInvScaleX),
		aRawRange.getMaxY() - (GetTextLowerDistance() * fAbsInvScaleY));
}
void SdrObjCustomShape::TakeTextRange(SdrOutliner& rOutliner, basegfx::B2DRange& rTextRange, basegfx::B2DRange& rAnchorRange) const
{
	// get TextRange without shear, rotate and mirror, just scaled
	// and centered in logic coordinates
	rAnchorRange = getScaledCenteredTextRange(*this);

	// Rect innerhalb dem geankert wird
	SdrTextVertAdjust eVAdj = GetTextVerticalAdjust();
	SdrTextHorzAdjust eHAdj = GetTextHorizontalAdjust();
	sal_uInt32 nStat0 = rOutliner.GetControlWord();
	Size aNullSize;

	rOutliner.SetControlWord(nStat0|EE_CNTRL_AUTOPAGESIZE);
	rOutliner.SetMinAutoPaperSize(aNullSize);

	sal_Int32 nMaxAutoPaperWidth = 1000000;
	sal_Int32 nMaxAutoPaperHeight= 1000000;

	long nAnkWdt(basegfx::fround(rAnchorRange.getWidth()));
	long nAnkHgt(basegfx::fround(rAnchorRange.getHeight()));

	if(((SdrOnOffItem&)(GetMergedItem(SDRATTR_TEXT_WORDWRAP))).GetValue())
	{
		if(IsVerticalWriting())
		{
			nMaxAutoPaperHeight = nAnkHgt;
		}
		else
		{
			nMaxAutoPaperWidth = nAnkWdt;
		}
	}

	if(SDRTEXTHORZADJUST_BLOCK == eHAdj && !IsVerticalWriting())
	{
		rOutliner.SetMinAutoPaperSize(Size(nAnkWdt, 0));
	}

	if(SDRTEXTVERTADJUST_BLOCK == eVAdj && IsVerticalWriting())
	{
		rOutliner.SetMinAutoPaperSize(Size(0, nAnkHgt));
	}

	rOutliner.SetMaxAutoPaperSize( Size( nMaxAutoPaperWidth, nMaxAutoPaperHeight ) );
	rOutliner.SetPaperSize( aNullSize );

	// Text in den Outliner stecken - ggf. den aus dem EditOutliner
	OutlinerParaObject* pPara = GetOutlinerParaObject();
	bool bNeedToDestroy(false);

	if(pEdtOutl)
	{
		pPara = pEdtOutl->CreateParaObject();
		bNeedToDestroy = true;
	}

	if (pPara)
	{
		const SdrTextObj* pTestObj = rOutliner.GetTextObj();

		if( !pTestObj || pTestObj != this ||
		    pTestObj->GetOutlinerParaObject() != GetOutlinerParaObject() )
		{
			rOutliner.SetTextObj( this );
			rOutliner.SetUpdateMode(true);
			rOutliner.SetText(*pPara);
		}
	}
	else
	{
		rOutliner.SetTextObj( NULL );
	}

	if (bNeedToDestroy && pPara)
	{
		delete pPara;
	}

	rOutliner.SetUpdateMode(true);
	rOutliner.SetControlWord(nStat0);

	SdrText* pText = getActiveText();

	if( pText )
	{
		pText->CheckPortionInfo( rOutliner );
	}

	basegfx::B2DPoint aTextPos(rAnchorRange.getMinimum());
	const basegfx::B2DVector aTextSiz(rOutliner.GetPaperSize().getWidth(), rOutliner.GetPaperSize().getHeight());

	// #106653#
	// For draw objects containing text correct hor/ver alignment if text is bigger
	// than the object itself. Without that correction, the text would always be
		// formatted to the left edge (or top edge when vertical) of the draw object.

	if( !IsTextFrame() )
	{
		if(rAnchorRange.getWidth() < aTextSiz.getX() && !IsVerticalWriting())
		{
			// #110129#
			// Horizontal case here. Correct only if eHAdj == SDRTEXTHORZADJUST_BLOCK,
			// else the alignment is wanted.
			if(SDRTEXTHORZADJUST_BLOCK == eHAdj)
			{
				eHAdj = SDRTEXTHORZADJUST_CENTER;
			}
		}

		if(rAnchorRange.getHeight() < aTextSiz.getY() && IsVerticalWriting())
		{
			// #110129#
			// Vertical case here. Correct only if eHAdj == SDRTEXTVERTADJUST_BLOCK,
			// else the alignment is wanted.
			if(SDRTEXTVERTADJUST_BLOCK == eVAdj)
			{
				eVAdj = SDRTEXTVERTADJUST_CENTER;
			}
		}
	}

	if (SDRTEXTHORZADJUST_CENTER == eHAdj || SDRTEXTHORZADJUST_RIGHT == eHAdj)
	{
		const double fFreeWdt(rAnchorRange.getWidth() - aTextSiz.getX());
		
		if(SDRTEXTHORZADJUST_CENTER == eHAdj)
		{
			aTextPos.setX(aTextPos.getX() + (fFreeWdt * 0.5));
		}
		else if(SDRTEXTHORZADJUST_RIGHT == eHAdj)
		{
			aTextPos.setX(aTextPos.getX() + fFreeWdt);
		}
	}

	if (SDRTEXTVERTADJUST_CENTER == eVAdj || SDRTEXTVERTADJUST_BOTTOM == eVAdj)
	{
		const double fFreeHgt(rAnchorRange.getHeight() - aTextSiz.getY());

		if(SDRTEXTVERTADJUST_CENTER == eVAdj)
		{
			aTextPos.setY(aTextPos.getY() + (fFreeHgt * 0.5));
		}
		else if(SDRTEXTVERTADJUST_BOTTOM == eVAdj)
		{
			aTextPos.setY(aTextPos.getY() + fFreeHgt);
		}
	}

	rTextRange = basegfx::B2DRange(aTextPos, aTextPos + aTextSiz);
}

void SdrObjCustomShape::SetChanged()
{
	// call parent
	SdrTextObj::SetChanged();

	// own reactions
	InvalidateRenderGeometry();
}

void SdrObjCustomShape::TakeObjNameSingul(XubString& rName) const
{
	rName = ImpGetResStr(STR_ObjNameSingulCUSTOMSHAPE);
	String aNm( GetName() );
	if( aNm.Len() )
	{
		rName += sal_Unicode(' ');
		rName += sal_Unicode('\'');
		rName += aNm;
		rName += sal_Unicode('\'');
	}
}

void SdrObjCustomShape::TakeObjNamePlural(XubString& rName) const
{
	rName=ImpGetResStr(STR_ObjNamePluralCUSTOMSHAPE);
}

basegfx::B2DPolyPolygon SdrObjCustomShape::TakeXorPoly() const
{
	return GetLineGeometry( (SdrObjCustomShape*)this, false );
}

SdrObject* SdrObjCustomShape::DoConvertToPolygonObject(bool bBezier, bool bAddText) const
{
	// #i37011#
	SdrObject* pRetval = 0L;
	SdrObject* pRenderedCustomShape = 0L;

	if ( !mXRenderedCustomShape.is() )
	{
		// force CustomShape
		((SdrObjCustomShape*)this)->GetSdrObjectFromCustomShape();
	}

	if ( mXRenderedCustomShape.is() )
	{
		pRenderedCustomShape = GetSdrObjectFromXShape( mXRenderedCustomShape );
	}

	if ( pRenderedCustomShape )
	{
		SdrObject* pCandidate = pRenderedCustomShape->CloneSdrObject();
		DBG_ASSERT(pCandidate, "SdrObjCustomShape::DoConvertToPolygonObject: Could not clone SdrObject (!)");
		pRetval = pCandidate->DoConvertToPolygonObject(bBezier, bAddText);
		deleteSdrObjectSafeAndClearPointer(pCandidate);

		if(pRetval)
		{
			const bool bShadow(((SdrOnOffItem&)GetMergedItem(SDRATTR_SHADOW)).GetValue());
			if(bShadow)
			{
				pRetval->SetMergedItem(SdrOnOffItem(SDRATTR_SHADOW, true));
			}
		}

		if(bAddText && HasText() && !IsTextPath())
		{
			pRetval = ImpConvertAddText(pRetval, bBezier);
		}
	}

	return pRetval;
}

void SdrObjCustomShape::SetStyleSheet( SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr )
{
	// #i40944#
	InvalidateRenderGeometry();
	
	// call parent
	SdrObject::SetStyleSheet( pNewStyleSheet, bDontRemoveHardAttr );
}

void SdrObjCustomShape::handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage)
{
	if(pOldPage != pNewPage)
	{
		// call parent
		SdrTextObj::handlePageChange(pOldPage, pNewPage);

		if( pNewPage )
		{
			// invalidating rectangles by invalidateObjectRange is not sufficient,
			// AdjustTextFrameWidthAndHeight() also has to be made
			ActionChanged();
			AdjustTextFrameWidthAndHeight();
		}
	}
}

SdrObjGeoData* SdrObjCustomShape::NewGeoData() const
{
	return new SdrAShapeObjGeoData;
}

void SdrObjCustomShape::SaveGeoData(SdrObjGeoData& rGeo) const
{
	SdrTextObj::SaveGeoData( rGeo );
	SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
	rAGeo.fObjectRotation = fObjectRotation;
	rAGeo.bMirroredX = IsMirroredX();
	rAGeo.bMirroredY = IsMirroredY();

	const rtl::OUString	sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
	Any* pAny( ( (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY ) ).GetPropertyValueByName( sAdjustmentValues ) );
	if ( pAny )
		*pAny >>= rAGeo.aAdjustmentSeq;
}

void SdrObjCustomShape::RestGeoData(const SdrObjGeoData& rGeo)
{
	SdrTextObj::RestGeoData( rGeo );
	SdrAShapeObjGeoData& rAGeo=(SdrAShapeObjGeoData&)rGeo;
	fObjectRotation = rAGeo.fObjectRotation;
	SetMirroredX( rAGeo.bMirroredX );
	SetMirroredY( rAGeo.bMirroredY );

	SdrCustomShapeGeometryItem rGeometryItem = (SdrCustomShapeGeometryItem&)GetMergedItem( SDRATTR_CUSTOMSHAPE_GEOMETRY );
	const rtl::OUString	sAdjustmentValues( RTL_CONSTASCII_USTRINGPARAM ( "AdjustmentValues" ) );
	PropertyValue aPropVal;
	aPropVal.Name = sAdjustmentValues;
	aPropVal.Value <<= rAGeo.aAdjustmentSeq;
	rGeometryItem.SetPropertyValue( aPropVal );
	SetMergedItem( rGeometryItem );

	InvalidateRenderGeometry();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sdr::contact::ViewContact* SdrObjCustomShape::CreateObjectSpecificViewContact()
{
	return new sdr::contact::ViewContactOfSdrObjCustomShape(*this);
}

// #i33136#
bool SdrObjCustomShape::doConstructOrthogonal(const ::rtl::OUString& rName)
{
	bool bRetval(false);
	static ::rtl::OUString Imps_sNameASOrtho_quadrat( RTL_CONSTASCII_USTRINGPARAM( "quadrat" ) );
	static ::rtl::OUString Imps_sNameASOrtho_round_quadrat( RTL_CONSTASCII_USTRINGPARAM( "round-quadrat" ) );
	static ::rtl::OUString Imps_sNameASOrtho_circle( RTL_CONSTASCII_USTRINGPARAM( "circle" ) );
	static ::rtl::OUString Imps_sNameASOrtho_circle_pie( RTL_CONSTASCII_USTRINGPARAM( "circle-pie" ) );
	static ::rtl::OUString Imps_sNameASOrtho_ring( RTL_CONSTASCII_USTRINGPARAM( "ring" ) );

	if(Imps_sNameASOrtho_quadrat.equalsIgnoreAsciiCase(rName))
	{
		bRetval = true;
	}
	else if(Imps_sNameASOrtho_round_quadrat.equalsIgnoreAsciiCase(rName))
	{
		bRetval = true;
	}
	else if(Imps_sNameASOrtho_circle.equalsIgnoreAsciiCase(rName))
	{
		bRetval = true;
	}
	else if(Imps_sNameASOrtho_circle_pie.equalsIgnoreAsciiCase(rName))
	{
		bRetval = true;
	}
	else if(Imps_sNameASOrtho_ring.equalsIgnoreAsciiCase(rName))
	{
		bRetval = true;
	}

	return bRetval;
}

// #i37011# centralize throw-away of render geometry
void SdrObjCustomShape::InvalidateRenderGeometry()
{
	mXRenderedCustomShape = 0L;
    deleteSdrObjectSafeAndClearPointer( mpLastShadowGeometry );
	mpLastShadowGeometry = 0L;
}

// eof
