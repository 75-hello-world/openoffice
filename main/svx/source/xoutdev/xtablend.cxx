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

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>

#ifndef _SV_APP_HXX
#include <vcl/svapp.hxx>
#endif
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/xlnwtit.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtLineEnd[]	= {'s','o','e'};
//static char const aChckLEnd[]  = { 0x04, 0x00, 'S','O','E','L'};	// < 5.2
//static char const aChckLEnd0[] = { 0x04, 0x00, 'S','O','E','0'};	// = 5.2
//static char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// --------------------
// class XLineEndList
// --------------------

class impXLineEndList
{
private:
	VirtualDevice*          mpVirtualDevice;
	SdrModel*				mpSdrModel;
	SdrObject*			    mpBackgroundObject;
	SdrObject*			    mpLineObject;

public:
    impXLineEndList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB, SdrObject* pL)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB),
        mpLineObject(pL)
    {}

    ~impXLineEndList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        SdrObject::Free(mpLineObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
    SdrObject* getLineObject() const { return mpLineObject; }
};

void XLineEndList::impCreate()
{
    if(!mpData)
    {
    	const Point aZero(0, 0);
		const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
		OSL_ENSURE(0 != pVirDev, "XLineEndList: no VirtualDevice created!" );
		pVirDev->SetMapMode(MAP_100TH_MM);
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
		const Size aSize(pVirDev->PixelToLogic(Size(rSize.Width() * 2, rSize.Height())));
		pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        pVirDev->SetBackground(rStyleSettings.GetFieldColor());

	    SdrModel* pSdrModel = new SdrModel();
		OSL_ENSURE(0 != pSdrModel, "XLineEndList: no SdrModel created!" );
	    pSdrModel->GetItemPool().FreezeIdRanges();

        const Rectangle aBackgroundSize(aZero, aSize);
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
		OSL_ENSURE(0 != pBackgroundObject, "XLineEndList: no BackgroundObject created!" );
    	pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_NONE));
        pBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));

        const basegfx::B2DPoint aStart(0, aSize.Height() / 2);
        const basegfx::B2DPoint aEnd(aSize.Width(), aSize.Height() / 2);
	    basegfx::B2DPolygon aPolygon;
	    aPolygon.append(aStart);
	    aPolygon.append(aEnd);
	    SdrObject* pLineObject = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPolygon));
		OSL_ENSURE(0 != pLineObject, "XLineEndList: no LineObject created!" );
    	pLineObject->SetModel(pSdrModel);
		const Size aLineWidth(pVirDev->PixelToLogic(Size(rStyleSettings.GetListBoxPreviewDefaultLineWidth(), 0)));
        pLineObject->SetMergedItem(XLineWidthItem(aLineWidth.getWidth()));
        const sal_uInt32 nArrowHeight((aSize.Height() * 8) / 10);
        pLineObject->SetMergedItem(XLineStartWidthItem(nArrowHeight));
        pLineObject->SetMergedItem(XLineEndWidthItem(nArrowHeight));
        pLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));

        mpData = new impXLineEndList(pVirDev, pSdrModel, pBackgroundObject, pLineObject);
		OSL_ENSURE(0 != mpData, "XLineEndList: data creation went wrong!" );
    }
}

void XLineEndList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XLineEndList::XLineEndList(const String& rPath, XOutdevItemPool* _pXPool)
:	XPropertyList(rPath, _pXPool),
	mpData(0)
{
}

XLineEndList::~XLineEndList()
{
    impDestroy();
}

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
	return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
	return (XLineEndEntry*) XPropertyList::Remove(nIndex, 0);
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
	return (XLineEndEntry*) XPropertyList::Get(nIndex, 0);
}

sal_Bool XLineEndList::Load()
{
	if( mbListDirty )
	{
		mbListDirty = false;

		INetURLObject aURL( maPath );

		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
		{
			DBG_ASSERT( !maPath.Len(), "invalid URL" );
			return sal_False;
		}

		aURL.Append( maName );

		if( !aURL.getExtension().getLength() )
			aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

		uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
	}
	return( sal_False );
}

sal_Bool XLineEndList::Save()
{
	INetURLObject aURL( maPath );

	if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
	{
		DBG_ASSERT( !maPath.Len(), "invalid URL" );
		return sal_False;
	}

	aURL.Append( maName );

	if( !aURL.getExtension().getLength() )
		aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

	uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
	return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XLineEndList::Create()
{
	basegfx::B2DPolygon aTriangle;
	aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
	aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
	aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
	aTriangle.setClosed(true);
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aTriangle), SVX_RESSTR( RID_SVXSTR_ARROW ) ) );

	basegfx::B2DPolygon aSquare;
	aSquare.append(basegfx::B2DPoint(0.0, 0.0));
	aSquare.append(basegfx::B2DPoint(10.0, 0.0));
	aSquare.append(basegfx::B2DPoint(10.0, 10.0));
	aSquare.append(basegfx::B2DPoint(0.0, 10.0));
	aSquare.setClosed(true);
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aSquare), SVX_RESSTR( RID_SVXSTR_SQUARE ) ) );

	basegfx::B2DPolygon aCircle(basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 100.0));
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aCircle), SVX_RESSTR( RID_SVXSTR_CIRCLE ) ) );

	return( sal_True );
}

Bitmap XLineEndList::CreateBitmapForUI( long nIndex )
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pLine = mpData->getLineObject();

    pLine->SetMergedItem(XLineStyleItem(XLINE_SOLID));
    pLine->SetMergedItem(XLineStartItem(String(), GetLineEnd(nIndex)->GetLineEnd()));
    pLine->SetMergedItem(XLineEndItem(String(), GetLineEnd(nIndex)->GetLineEnd()));

    sdr::contact::SdrObjectVector aObjectVector;
	aObjectVector.push_back(mpData->getBackgroundObject());
	aObjectVector.push_back(pLine);
	sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
	sdr::contact::DisplayInfo aDisplayInfo;

    pVD->Erase();
	aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
	return pVD->GetBitmap(aZero, pVD->GetOutputSize());
}

//////////////////////////////////////////////////////////////////////////////
// eof
