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
#include "precompiled_sd.hxx"
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <vos/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#ifndef _UNTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <sot/storage.hxx>
#include <svl/itempool.hxx>
#include <editeng/editobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <tools/urlobj.hxx>
#include <sot/formats.hxx>
#include <svl/urlbmk.hxx>
#include <editeng/outliner.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <unotools/streamwrap.hxx>

#include <svx/svdotable.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedtransfer.hxx>
#include "DrawDocShell.hxx"
#include "View.hxx"
#include "sdpage.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "imapinfo.hxx"
#include "sdxfer.hxx"
#include "unomodel.hxx"
#include <vcl/virdev.hxx>
#include <svx/svdlegacy.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

// -----------
// - Defines -
// -----------

#define SDTRANSFER_OBJECTTYPE_DRAWMODEL			0x00000001
#define SDTRANSFER_OBJECTTYPE_DRAWOLE			0x00000002

// ------------------
// - SdTransferable -
// ------------------

SdTransferable::SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, bool bInitOnGetData )
:	mpPageDocShell( NULL )
,	mpOLEDataHelper( NULL )
,	mpObjDesc( NULL )
,	mpSdView( pWorkView )
,	mpSdViewIntern( pWorkView )
,	mpSdDrawDocument( NULL )
,	mpSdDrawDocumentIntern( NULL )
,	mpSourceDoc( pSrcDoc )
,	mpVDev( NULL )
,	mpBookmark( NULL )
,	mpGraphic( NULL )
,	mpImageMap( NULL )
,	mbInternalMove( false )
,	mbOwnDocument( false )
,	mbOwnView( false )
,	mbLateInit( bInitOnGetData )
,	mbPageTransferable( false )
,	mbPageTransferablePersistent( false )
,	mbIsUnoObj( false )
{
	if( mpSourceDoc )
		StartListening( *mpSourceDoc );

	if( pWorkView )
		StartListening( *pWorkView );

    if( !mbLateInit )
	    CreateData();
}

// -----------------------------------------------------------------------------

SdTransferable::~SdTransferable()
{
	if( mpSourceDoc )
		EndListening( *mpSourceDoc );

	if( mpSdView )
		EndListening( *const_cast< sd::View *>( mpSdView) );

	Application::GetSolarMutex().acquire();

    ObjectReleased();

    for( void* p = maPageBookmarks.First(); p; p = maPageBookmarks.Next() )
        delete static_cast< String* >( p );

	if( mbOwnView )
		delete mpSdViewIntern;

    delete mpOLEDataHelper;

	if( maDocShellRef.Is() )
	{
        SfxObjectShell* pObj = maDocShellRef;
		::sd::DrawDocShell* pDocSh = static_cast< ::sd::DrawDocShell*>(pObj);
		pDocSh->DoClose();
	}

	maDocShellRef.Clear();

	if( mbOwnDocument )
		delete mpSdDrawDocumentIntern;

	delete mpGraphic;
	delete mpBookmark;
	delete mpImageMap;

	delete mpVDev;
	delete mpObjDesc;

	Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateObjectReplacement( SdrObject* pObj )
{
	if( pObj )
	{
        delete mpOLEDataHelper, mpOLEDataHelper = NULL;
		delete mpGraphic, mpGraphic = NULL;
		delete mpBookmark, mpBookmark = NULL;
		delete mpImageMap, mpImageMap = NULL;
		SdrOle2Obj* pSdrOle2Obj = dynamic_cast< SdrOle2Obj* >(pObj);
		
		if( pSdrOle2Obj )
		{
			try
			{
            	uno::Reference < embed::XEmbeddedObject > xObj = pSdrOle2Obj->GetObjRef();
				uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
            	if( xObj.is() && xPersist.is() && xPersist->hasEntry() )
				{
                	mpOLEDataHelper = new TransferableDataHelper( new SvEmbedTransferHelper( xObj, pSdrOle2Obj->GetGraphic(), pSdrOle2Obj->GetAspect() ) );

					// TODO/LATER: the standalone handling of the graphic should not be used any more in future
					// The EmbedDataHelper should bring the graphic in future
					Graphic* pObjGr = pSdrOle2Obj->GetGraphic();
					if ( pObjGr )
						mpGraphic = new Graphic( *pObjGr );
				}
			}
			catch( uno::Exception& )
			{}
		}
		else 
		{
			SdrGrafObj* pSdrGrafObj = dynamic_cast< SdrGrafObj* >(pObj);

			if( pSdrGrafObj && (mpSourceDoc && !mpSourceDoc->GetAnimationInfo( pObj )) )
			{
				mpGraphic = new Graphic( pSdrGrafObj->GetTransformedGraphic() );
			}
			else 
			{
				SdrUnoObj* pUnoCtrl = dynamic_cast< SdrUnoObj* >( pObj );

				if( pUnoCtrl && FmFormInventor == pObj->GetObjInventor() && ( pObj->GetObjIdentifier() == (sal_uInt16) OBJ_FM_BUTTON ) )
				{
					if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
					{
						Reference< ::com::sun::star::awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

						if( !xControlModel.is() )
							return;

						Reference< ::com::sun::star::beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY );

						if( !xPropSet.is() )
							return;

						::com::sun::star::form::FormButtonType	eButtonType;
						Any										aTmp( xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ) ) ) );

						if( aTmp >>= eButtonType )
						{
							::rtl::OUString aLabel, aURL;

							xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ) ) ) >>= aLabel;
							xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TargetURL") ) ) >>= aURL;

							mpBookmark = new INetBookmark( String( aURL ), String( aLabel ) );
						}
					}
				}
				else 
				{
					SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pObj);

					if( pSdrTextObj )
					{
						const OutlinerParaObject* pPara;

						if( (pPara = pSdrTextObj->GetOutlinerParaObject()) != 0 )
						{
							const SvxFieldItem* pField;

							if( (pField = pPara->GetTextObject().GetField()) != 0 )
							{
								const SvxURLField* pURL = dynamic_cast< const SvxURLField* >(pField->GetField());

								if( pURL )
								{
									mpBookmark = new INetBookmark( pURL->GetURL(), pURL->GetRepresentation() );
								}
							}
						}
					}
				}
			}
		}

		SdDrawDocument&	rSdDrawDocument = dynamic_cast< SdDrawDocument& >( pObj->getSdrModelFromSdrObject() );
		SdIMapInfo*	pInfo = rSdDrawDocument.GetIMapInfo(pObj);

		if( pInfo )
			mpImageMap = new ImageMap( pInfo->GetImageMap() );

		mbIsUnoObj = pObj && pObj->IsSdrUnoObj();
	}
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateData()
{
	if( mpSdDrawDocument && !mpSdViewIntern )
	{
		mbOwnView = true;

		SdPage* pPage = mpSdDrawDocument->GetSdPage(0, PK_STANDARD);

		if( 1 == pPage->GetObjCount() )
			CreateObjectReplacement( pPage->GetObj( 0 ) );

		mpVDev = new VirtualDevice( *Application::GetDefaultDevice() );
		mpVDev->SetMapMode( MapMode( mpSdDrawDocumentIntern->GetExchangeObjectUnit(), Point(), mpSdDrawDocumentIntern->GetExchangeObjectScale(), mpSdDrawDocumentIntern->GetExchangeObjectScale() ) );
		mpSdViewIntern = new ::sd::View( *mpSdDrawDocumentIntern, mpVDev );
		mpSdViewIntern->EndListening(*mpSdDrawDocumentIntern );
		mpSdViewIntern->ShowSdrPage(*pPage);
		((SdrMarkView*)mpSdViewIntern)->MarkAllObj();
	}
	else if( mpSdView && !mpSdDrawDocumentIntern )
	{
		SdrObject* pSelected(mpSdView->getSelectedIfSingle());

		if( pSelected )
			CreateObjectReplacement( pSelected );

		if( mpSourceDoc )
			mpSourceDoc->CreatingDataObj(this);
		mpSdDrawDocumentIntern = (SdDrawDocument*) mpSdView->GetAllMarkedModel();
		if( mpSourceDoc )
			mpSourceDoc->CreatingDataObj(0);

		if( !maDocShellRef.Is() && mpSdDrawDocumentIntern->GetDocSh() )
			maDocShellRef = mpSdDrawDocumentIntern->GetDocSh();

		if( !maDocShellRef.Is() )
		{
			DBG_ERROR( "SdTransferable::CreateData(), failed to create a model with persist, clipboard operation will fail for OLE objects!" );
			mbOwnDocument = true;
		}

		// Groesse der Source-Seite uebernehmen
		SdrPageView*		pPgView = mpSdView->GetSdrPageView();
		SdPage*				pOldPage = pPgView ? (SdPage*) &pPgView->getSdrPageFromSdrPageView() : 0;
		SdStyleSheetPool*	pOldStylePool = (SdStyleSheetPool*) mpSdView->getSdrModelFromSdrView().GetStyleSheetPool();
		SdStyleSheetPool*	pNewStylePool = (SdStyleSheetPool*) mpSdDrawDocumentIntern->GetStyleSheetPool();
		SdPage*				pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );
		String				aOldLayoutName( pOldPage->GetLayoutName() );

		pPage->SetPageScale( pOldPage->GetPageScale() );
		pPage->SetLayoutName( aOldLayoutName );
		pNewStylePool->CopyGraphicSheets( *pOldStylePool );
		pNewStylePool->CopyCellSheets( *pOldStylePool );
		pNewStylePool->CopyTableStyles( *pOldStylePool );
		aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
		SdStyleSheetVector aCreatedSheets;
		pNewStylePool->CopyLayoutSheets( aOldLayoutName, *pOldStylePool, aCreatedSheets );
	}

	// set VisArea and adjust objects if neccessary
	if( maVisArea.IsEmpty() &&
		mpSdDrawDocumentIntern && mpSdViewIntern &&
		mpSdDrawDocumentIntern->GetPageCount() )
	{
		SdPage*	pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );

		if(1 == mpSdDrawDocumentIntern->GetPageCount())
		{
            const basegfx::B2DRange aAllRange(mpSdViewIntern->getMarkedObjectSnapRange());
            const basegfx::B2DHomMatrix aTranslate(
                basegfx::tools::createTranslateB2DHomMatrix(
                    -aAllRange.getMinimum()));

			for(sal_uInt32 nObj(0), nObjCount = pPage->GetObjCount(); nObj < nObjCount; nObj++)
			{
				SdrObject* pObj = pPage->GetObj(nObj);

                sdr::legacy::transformSdrObject(*pObj, aTranslate);
			}

            maVisArea.SetSize(Size(basegfx::fround(aAllRange.getWidth()), basegfx::fround(aAllRange.getHeight())));
		}
		else
		{
			const basegfx::B2DVector& rPageScale = pPage->GetPageScale();

            maVisArea.SetSize(Size(basegfx::fround(rPageScale.getX()), basegfx::fround(rPageScale.getY())));
		}

		// Die Ausgabe soll am Nullpunkt erfolgen
		maVisArea.SetPos(Point());
	}
}

// -----------------------------------------------------------------------------

bool lcl_HasOnlyControls( SdrModel* pModel )
{
    bool bOnlyControls = false;         // default if there are no objects

    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObj = aIter.Next();
            if ( pObj )
            {
                bOnlyControls = true;   // only set if there are any objects at all
                while ( pObj )
                {
                    if (!dynamic_cast< SdrUnoObj* >(pObj))
                    {
                        bOnlyControls = false;
                        break;
                    }
                    pObj = aIter.Next();
                }
            }
        }
    }

    return bOnlyControls;
}

// -----------------------------------------------------------------------------

bool lcl_HasOnlyOneTable( SdrModel* pModel )
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
			if( dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) ) != 0 )
				return true;
		}
	}
	return false;
}

// -----------------------------------------------------------------------------

void SdTransferable::AddSupportedFormats()
{
    if( !mbPageTransferable || mbPageTransferablePersistent )
    {
        if( !mbLateInit )
            CreateData();

	    if( mpObjDesc )
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        if( mpOLEDataHelper )
	    {
		    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

		    DataFlavorExVector				aVector( mpOLEDataHelper->GetDataFlavorExVector() );
		    DataFlavorExVector::iterator	aIter( aVector.begin() ), aEnd( aVector.end() );

		    while( aIter != aEnd )
			    AddFormat( *aIter++ );
	    }
	    else if( mpGraphic )
	    {
			// #i25616#
		    AddFormat( SOT_FORMATSTR_ID_DRAWING );

			AddFormat( SOT_FORMATSTR_ID_SVXB );

		    if( mpGraphic->GetType() == GRAPHIC_BITMAP )
		    {
			    AddFormat( SOT_FORMAT_BITMAP );
			    AddFormat( SOT_FORMAT_GDIMETAFILE );
		    }
		    else
		    {
			    AddFormat( SOT_FORMAT_GDIMETAFILE );
			    AddFormat( SOT_FORMAT_BITMAP );
		    }
	    }
	    else if( mpBookmark )
	    {
		    AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
		    AddFormat( FORMAT_STRING );
	    }
	    else
	    {
		    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
		    AddFormat( SOT_FORMATSTR_ID_DRAWING );
			if( !mpSdDrawDocument || !lcl_HasOnlyControls( mpSdDrawDocument ) )
			{
				AddFormat( SOT_FORMAT_GDIMETAFILE );
				AddFormat( SOT_FORMAT_BITMAP );
			}

			if( lcl_HasOnlyOneTable( mpSdDrawDocument ) )
				AddFormat( SOT_FORMAT_RTF );
	    }

	    if( mpImageMap )
		    AddFormat( SOT_FORMATSTR_ID_SVIM );
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdTransferable::GetData( const DataFlavor& rFlavor )
{
	if (SD_MOD()==NULL)
		return sal_False;

	sal_uInt32	nFormat = SotExchange::GetFormat( rFlavor );
	sal_Bool	bOK = sal_False;

    CreateData();

	if( nFormat == SOT_FORMAT_RTF && lcl_HasOnlyOneTable( mpSdDrawDocument ) )
	{
		bOK = SetTableRTF( mpSdDrawDocument, rFlavor );
	}
	else if( mpOLEDataHelper && mpOLEDataHelper->HasFormat( rFlavor ) )
	{
		sal_uLong nOldSwapMode = 0;

		if( mpSdDrawDocumentIntern )
		{
			nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
			mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
		}

		// TODO/LATER: support all the graphical formats, the embedded object scenario should not have separated handling
		if( nFormat == FORMAT_GDIMETAFILE && mpGraphic )
			bOK = SetGDIMetaFile( mpGraphic->GetGDIMetaFile(), rFlavor );
		else
			bOK = SetAny( mpOLEDataHelper->GetAny( rFlavor ), rFlavor );

		if( mpSdDrawDocumentIntern )
			mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
	}
	else if( HasFormat( nFormat ) )
	{
		if( ( nFormat == SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR || nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) && mpObjDesc )
		{
			bOK = SetTransferableObjectDescriptor( *mpObjDesc, rFlavor );
		}
		else if( nFormat == SOT_FORMATSTR_ID_DRAWING )
		{
			SfxObjectShellRef aOldRef( maDocShellRef );

			maDocShellRef.Clear();

			if( mpSdViewIntern )
			{
				SdDrawDocument* pInternDoc = mpSdViewIntern->GetDoc();
				if( pInternDoc )
					pInternDoc->CreatingDataObj(this);
				SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( mpSdViewIntern->GetAllMarkedModel() );
				if( pInternDoc )
					pInternDoc->CreatingDataObj(0);

				bOK = SetObject( pDoc, SDTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );

				if( maDocShellRef.Is() )
				{
					maDocShellRef->DoClose();
				}
				else
				{
					delete pDoc;
				}
			}

			maDocShellRef = aOldRef;
		}
		else if( nFormat == FORMAT_GDIMETAFILE )
		{
			if( mpSdViewIntern )
				bOK = SetGDIMetaFile(mpSdViewIntern->GetMarkedObjMetaFile(true), rFlavor);
		}
		else if( nFormat == FORMAT_BITMAP )
		{
			if( mpSdViewIntern )
				bOK = SetBitmap(mpSdViewIntern->GetMarkedObjBitmapEx(true).GetBitmap(), rFlavor);
		}
		else if( ( nFormat == FORMAT_STRING ) && mpBookmark )
		{
			bOK = SetString( mpBookmark->GetURL(), rFlavor );
		}
		else if( ( nFormat == SOT_FORMATSTR_ID_SVXB ) && mpGraphic )
		{
			bOK = SetGraphic( *mpGraphic, rFlavor );
		}
		else if( ( nFormat == SOT_FORMATSTR_ID_SVIM ) && mpImageMap )
		{
			bOK = SetImageMap( *mpImageMap, rFlavor );
		}
		else if( mpBookmark )
		{
			bOK = SetINetBookmark( *mpBookmark, rFlavor );
		}
		else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
		{
			sal_uLong nOldSwapMode = 0;

			if( mpSdDrawDocumentIntern )
			{
				nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
				mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
			}

			if( !maDocShellRef.Is() )
			{
				maDocShellRef = new ::sd::DrawDocShell(
                    mpSdDrawDocumentIntern,
                    SFX_CREATE_MODE_EMBEDDED,
                    true,
                    mpSdDrawDocumentIntern->GetDocumentType());
				mbOwnDocument = false;
				maDocShellRef->DoInitNew( NULL );
			}

			maDocShellRef->SetVisArea( maVisArea );
			bOK = SetObject( &maDocShellRef, SDTRANSFER_OBJECTTYPE_DRAWOLE, rFlavor );

			if( mpSdDrawDocumentIntern )
				mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
		}
	}

	return bOK;
}

// -----------------------------------------------------------------------------

sal_Bool SdTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pObject, sal_uInt32 nObjectType, const DataFlavor& )
{
	sal_Bool bRet = sal_False;

	switch( nObjectType )
	{
		case( SDTRANSFER_OBJECTTYPE_DRAWMODEL ):
		{
			try
			{
				static const bool bDontBurnInStyleSheet = ( getenv( "AVOID_BURN_IN_FOR_GALLERY_THEME" ) != NULL );
				SdDrawDocument* pDoc = (SdDrawDocument*) pObject;
				if ( !bDontBurnInStyleSheet )
					pDoc->BurnInStyleSheetAttributes();
				rxOStm->SetBufferSize( 16348 );

				Reference< XComponent > xComponent( new SdXImpressDocument( pDoc, sal_True ) );
				pDoc->setUnoModel( Reference< XInterface >::query( xComponent ) );

				{
					com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
					if( SvxDrawingLayerExport( pDoc, xDocOut, xComponent, (pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? "com.sun.star.comp.Impress.XMLClipboardExporter" : "com.sun.star.comp.DrawingLayer.XMLExporter" ) )
						rxOStm->Commit();
				}

				xComponent->dispose();
				bRet = ( rxOStm->GetError() == ERRCODE_NONE );
			}
			catch( Exception& )
			{
				DBG_ERROR( "sd::SdTransferable::WriteObject(), exception catched!" );
				bRet = false;
			}
		}
		break;

		case( SDTRANSFER_OBJECTTYPE_DRAWOLE ):
		{
            SfxObjectShell*   pEmbObj = (SfxObjectShell*) pObject;
            ::utl::TempFile     aTempFile;
            aTempFile.EnableKillingFile();

            try
            {
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, sal_False );
                // mba: no relative ULRs for clipboard!
                SfxMedium aMedium( xWorkStore, String() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    *rxOStm << *pSrcStm;
                    delete pSrcStm;
                }

                bRet = true;
                rxOStm->Commit();
            }
            catch ( Exception& )
            {}
		}

		break;

		default:
		break;
	}

	return bRet;
}

// -----------------------------------------------------------------------------

void SdTransferable::DragFinished( sal_Int8 nDropAction )
{
	if( mpSdView )
		( (::sd::View*) mpSdView )->DragFinished( nDropAction );
}

// -----------------------------------------------------------------------------

void SdTransferable::ObjectReleased()
{
	if( this == SD_MOD()->pTransferClip )
		SD_MOD()->pTransferClip = NULL;

	if( this == SD_MOD()->pTransferDrag )
		SD_MOD()->pTransferDrag = NULL;

	if( this == SD_MOD()->pTransferSelection )
		SD_MOD()->pTransferSelection = NULL;
}

// -----------------------------------------------------------------------------

void SdTransferable::SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc )
{
	delete mpObjDesc;
	mpObjDesc = new TransferableObjectDescriptor( rObjDesc );
    PrepareOLE( rObjDesc );
}

// -----------------------------------------------------------------------------

void SdTransferable::SetPageBookmarks( const List& rPageBookmarks, bool bPersistent )
{
    if( mpSourceDoc )
    {
	    if( mpSdViewIntern )
		    mpSdViewIntern->HideSdrPage();

		// #116168#
        mpSdDrawDocument->ClearModel(false);

		mpPageDocShell = NULL;

        for( void* p = maPageBookmarks.First(); p; p = maPageBookmarks.Next() )
            delete static_cast< String* >( p );

        if( bPersistent )
        {
            mpSdDrawDocument->CreateFirstPages(mpSourceDoc);
            mpSdDrawDocument->InsertBookmarkAsPage( const_cast< List* >( &rPageBookmarks ), NULL, false, true, 1, true, mpSourceDoc->GetDocSh(), true, true, false );
        }
        else
        {
            mpPageDocShell = mpSourceDoc->GetDocSh();

            for( sal_uLong i = 0; i < rPageBookmarks.Count(); i++ )
                maPageBookmarks.Insert( new String( *static_cast< String* >( rPageBookmarks.GetObject( i ) ) ), LIST_APPEND );
        }

	    if( mpSdViewIntern && mpSdDrawDocument )
	    {
		    SdPage* pPage = mpSdDrawDocument->GetSdPage( 0, PK_STANDARD );

		    if( pPage )
            {
				mpSdViewIntern->ShowSdrPage(*pPage);
			    mpSdViewIntern->MarkAllObj();
            }
	    }

        // don't offer any formats => it's just for internal puposes
        mbPageTransferable = true;
        mbPageTransferablePersistent = bPersistent;
    }
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL SdTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
		nRet = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
	{
        nRet = 0;
	}

	return nRet;
}

// -----------------------------------------------------------------------------

SdDrawDocument* SdTransferable::GetSourceDoc (void) const
{
    return mpSourceDoc;
}

// -----------------------------------------------------------------------------

void SdTransferable::AddUserData (const ::boost::shared_ptr<UserData>& rpData)
{
    maUserData.push_back(rpData);
}

// -----------------------------------------------------------------------------

void SdTransferable::RemoveUserData (const ::boost::shared_ptr<UserData>& rpData)
{
    maUserData.erase(::std::find(maUserData.begin(), maUserData.end(), rpData));
}

// -----------------------------------------------------------------------------

sal_Int32 SdTransferable::GetUserDataCount (void) const
{
    return maUserData.size();
}

// -----------------------------------------------------------------------------

::boost::shared_ptr<SdTransferable::UserData> SdTransferable::GetUserData (const sal_Int32 nIndex) const
{
    if (nIndex>=0 && nIndex<sal_Int32(maUserData.size()))
        return maUserData[nIndex];
    else
        return ::boost::shared_ptr<UserData>();
}

// -----------------------------------------------------------------------------

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdTransferable::getUnoTunnelId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;

	if( !aSeq.getLength() )
	{
		static osl::Mutex   aCreateMutex;
    	osl::MutexGuard     aGuard( aCreateMutex );

		aSeq.realloc( 16 );
    	rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
	}

    return aSeq;
}

// -----------------------------------------------------------------------------

SdTransferable* SdTransferable::getImplementation( const Reference< XInterface >& rxData ) throw()
{
    try
    {
		Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( rxData, UNO_QUERY_THROW );
		return reinterpret_cast<SdTransferable*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SdTransferable::getUnoTunnelId()) ) );
    }
    catch( const ::com::sun::star::uno::Exception& )
	{
	}
	return NULL;
}

// -----------------------------------------------------------------------------

// SfxListener
void SdTransferable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
	const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >( &rHint );

    if( pSdrHint )
	{
		if( HINT_MODELCLEARED == pSdrHint->GetSdrHintKind() )
		{
			EndListening(*mpSourceDoc);
			mpSourceDoc = 0;
		}
	}
	else
	{
		const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint * >(&rHint);
		if(pSimpleHint && (pSimpleHint->GetId() == SFX_HINT_DYING) )
		{
			if( &rBC == mpSourceDoc )
				mpSourceDoc = 0;
			if( &rBC == mpSdViewIntern )
				mpSdViewIntern = 0;
			if( &rBC == mpSdView )
				mpSdView = 0;
		}
	}
}

sal_Bool SdTransferable::SetTableRTF( SdDrawDocument* pModel, const DataFlavor& rFlavor)
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
			sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) );
			if( pTableObj )
			{
				SvMemoryStream aMemStm( 65535, 65535 );
				sdr::table::SdrTableObj::ExportAsRTF( aMemStm, *pTableObj );				
				return SetAny( Any( Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) ) ), rFlavor );
			}
		}
	}

	return sal_False;
}
