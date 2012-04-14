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

#include "DrawViewShell.hxx"
#include "ViewShellImplementation.hxx"
#include <vcl/waitobj.hxx>
#include <svx/svdograf.hxx>
#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#ifndef _ZOOMITEM_HXX
#include <svx/zoomitem.hxx>
#endif
#ifndef _EDITDATA_HXX
#include <editeng/editdata.hxx>
#endif
#include <basic/sberrors.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svl/aeitem.hxx>
#include <editeng/eeitem.hxx>
#include <basic/sbstar.hxx>
#include <editeng/flditem.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xfillit0.hxx>

#ifndef _SDOUTL_HXX //autogen
#include <svx/svdoutl.hxx>
#endif
#include <svx/xlnwtit.hxx>
#include <svx/svdoattr.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/fontworkbar.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>

#include <sfx2/viewfrm.hxx>
#include "sdgrffilter.hxx"

#include "app.hrc"
#include "glob.hrc"
#include "helpids.h"
#include "sdattr.hxx"
#include "drawview.hxx"
#include "Window.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "fuscale.hxx"
#include "sdresid.hxx"
#include "GraphicViewShell.hxx"
#include "unmodpg.hxx"
#include "slideshow.hxx"
#include "fuvect.hxx"
#include "stlpool.hxx"

// #90356#
#include "optsitem.hxx"
#include "sdabstdlg.hxx"
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <svx/svdlegacy.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer temporaere Funktionen
|*
\************************************************************************/

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
	// Waehrend einer Native-Diashow wird nichts ausgefuehrt!
	if(SlideShow::IsRunning( GetViewShellBase() ) && (rReq.GetSlot() != SID_NAVIGATOR))
		return;

	DBG_ASSERT( mpDrawView, "sd::DrawViewShell::FuTemporary(), no draw view!" );
	if( !mpDrawView )
		return;

	CheckLineTo (rReq);

	DeactivateCurrentFunction();

	sal_uInt16 nSId = rReq.GetSlot();

	// Slot wird gemapped (ToolboxImages/-Slots)
	MapSlot( nSId );

	switch ( nSId )
	{
		// Flaechen und Linien-Attribute:
		// Sollten (wie StateMethode) eine eigene
		// Execute-Methode besitzen
		case SID_ATTR_FILL_STYLE:
		case SID_ATTR_FILL_COLOR:
		case SID_ATTR_FILL_GRADIENT:
		case SID_ATTR_FILL_HATCH:
		case SID_ATTR_FILL_BITMAP:
		case SID_ATTR_FILL_SHADOW:

		case SID_ATTR_LINE_STYLE:
		case SID_ATTR_LINE_DASH:
		case SID_ATTR_LINE_WIDTH:
		case SID_ATTR_LINE_COLOR:
		case SID_ATTR_LINEEND_STYLE:

		case SID_ATTR_TEXT_FITTOSIZE:
		{
			if( rReq.GetArgs() )
			{
				bool bMergeUndo = false;
				::svl::IUndoManager* pUndoManager = GetDocSh()->GetUndoManager();

				// Anpassungen Start/EndWidth #63083#
				if(nSId == SID_ATTR_LINE_WIDTH)
				{
					const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
					const sal_Int32 nNewLineWidth = ((const XLineWidthItem&)rReq.GetArgs()->Get(XATTR_LINEWIDTH)).GetValue();

					for (sal_uInt32 i(0); i < aSelection.size(); i++)
					{
						SfxItemSet aAttr(GetDoc()->GetItemPool());
						SdrObject* pObj = aSelection[i];
						aAttr.Put(pObj->GetMergedItemSet());

						sal_Int32 nActLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();

						if(nActLineWidth != nNewLineWidth)
						{
							bool bSetItemSet(false);

							// #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
							if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINESTARTWIDTH))
							{
								sal_Int32 nValAct = ((const XLineStartWidthItem&)aAttr.Get(XATTR_LINESTARTWIDTH)).GetValue();
								sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
								if(nValNew < 0)
									nValNew = 0;
								bSetItemSet = true;
								aAttr.Put(XLineStartWidthItem(nValNew));
							}

							// #86265# do this for SFX_ITEM_DEFAULT and for SFX_ITEM_SET
							if(SFX_ITEM_DONTCARE != aAttr.GetItemState(XATTR_LINEENDWIDTH))
							{
								sal_Int32 nValAct = ((const XLineEndWidthItem&)aAttr.Get(XATTR_LINEENDWIDTH)).GetValue();
								sal_Int32 nValNew = nValAct + (((nNewLineWidth - nActLineWidth) * 15) / 10);
								if(nValNew < 0)
									nValNew = 0;
								bSetItemSet = true;
								aAttr.Put(XLineEndWidthItem(nValNew));
							}

							if(bSetItemSet)
								pObj->SetMergedItemSet(aAttr);
						}
					}
				}

				if (nSId == SID_ATTR_FILL_SHADOW)
				{
					// Ggf. werden transparente Objekte wei?gefuellt
					const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
					const bool bUndo = mpDrawView->IsUndoEnabled();

					for (sal_uInt32 i(0); i < aSelection.size(); i++)
					{
						SfxItemSet aAttr(GetDoc()->GetItemPool());
						SdrObject* pObj = aSelection[i];

						// #i25616#
						if(!static_cast< SdrGrafObj* >(pObj))
						{
							aAttr.Put(pObj->GetMergedItemSet());

							const XFillStyleItem& rFillStyle =
							(const XFillStyleItem&) aAttr.Get(XATTR_FILLSTYLE);

							if (rFillStyle.GetValue() == XFILL_NONE)
							{
								if( bUndo )
								{
									// Vorlage hat keine Fuellung,
									// daher hart attributieren: Fuellung setzen
									if (!bMergeUndo)
									{
										bMergeUndo = true;
										pUndoManager->EnterListAction( String(), String() );
										mpDrawView->BegUndo();
									}

									mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));
								}

								aAttr.Put(XFillStyleItem(XFILL_SOLID));
								aAttr.Put(XFillColorItem(String(), COL_WHITE));

								pObj->SetMergedItemSet(aAttr);
							}
						}
					}

					if (bMergeUndo)
					{
						mpDrawView->EndUndo();
					}
				}

				mpDrawView->SetAttributes(*rReq.GetArgs());

				if (bMergeUndo)
				{
					pUndoManager->LeaveListAction();
				}

				rReq.Done();
			}
			else
			{
				switch( rReq.GetSlot() )
				{
					case SID_ATTR_FILL_SHADOW:
					case SID_ATTR_FILL_STYLE:
					case SID_ATTR_FILL_COLOR:
					case SID_ATTR_FILL_GRADIENT:
					case SID_ATTR_FILL_HATCH:
					case SID_ATTR_FILL_BITMAP:
						GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SFX_CALLMODE_ASYNCHRON );
						break;
					case SID_ATTR_LINE_STYLE:
					case SID_ATTR_LINE_DASH:
					case SID_ATTR_LINE_WIDTH:
					case SID_ATTR_LINE_COLOR:
						GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SFX_CALLMODE_ASYNCHRON );
						break;
					case SID_ATTR_TEXT_FITTOSIZE:
						GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SFX_CALLMODE_ASYNCHRON );
						break;
				}
			}
			Cancel();
		}
		break;

		case SID_HYPHENATION:
		{
			// const SfxPoolItem* pItem = rReq.GetArg( SID_HYPHENATION );
			//  ^-- Soll so nicht benutzt werden (Defaults sind falsch) !
			SFX_REQUEST_ARG( rReq, pItem, SfxBoolItem, SID_HYPHENATION );

			if( pItem )
			{
				SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
				bool bValue = ( (const SfxBoolItem*) pItem)->GetValue();
				aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
				mpDrawView->SetAttributes( aSet );
			}
			else // nur zum Test
			{
				DBG_ERROR(" Kein Wert fuer Silbentrennung!");
				SfxItemSet aSet( GetPool(), EE_PARA_HYPHENATE, EE_PARA_HYPHENATE );
				bool bValue = true;
				aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
				mpDrawView->SetAttributes( aSet );
			}
			rReq.Done();
			Cancel();
		}
		break;

		case SID_INSERTPAGE:
		case SID_INSERTPAGE_QUICK:
		case SID_DUPLICATE_PAGE:
        {
            SdPage* pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            if (pNewPage != NULL)
                SwitchPage((pNewPage->GetPageNumber()-1)/2);
            rReq.Done ();
        }
		break;

		case SID_INSERT_MASTER_PAGE:
        {
            // Use the API to create a new page.
            Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier (
                GetDoc()->getUnoModel(), UNO_QUERY);
            if (xMasterPagesSupplier.is())
            {
                Reference<drawing::XDrawPages> xMasterPages (
                    xMasterPagesSupplier->getMasterPages());
                if (xMasterPages.is())
                {
                    sal_uInt32 nIndex = GetCurPageId();
                    xMasterPages->insertNewByIndex (nIndex);

                    // Create shapes for the default layout.
                    SdPage* pMasterPage = GetDoc()->GetMasterSdPage(
                        nIndex, PK_STANDARD);
                    pMasterPage->CreateTitleAndLayout (true,true);
                }
            }

            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            rReq.Done ();
        }
        break;

		case SID_MODIFYPAGE:
		{
			if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES ||
				(mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE) )
			{
				if ( mpDrawView->IsTextEdit() )
				{
					mpDrawView->SdrEndTextEdit();
				}
				sal_uInt32 nPage = maTabControl.GetCurPageId() - 1;
				mpActualPage = GetDoc()->GetSdPage(nPage, mePageKind);
                ::sd::ViewShell::mpImpl->ProcessModifyPageSlot (
                    rReq,
                    mpActualPage,
                    mePageKind);
			}

			Cancel();
			rReq.Done ();
		}
		break;

        case SID_ASSIGN_LAYOUT:
		{
			if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES || (mePageKind==PK_HANDOUT && meEditMode==EM_MASTERPAGE))
			{
			    if ( mpDrawView->IsTextEdit() )
				    mpDrawView->SdrEndTextEdit();

				::sd::ViewShell::mpImpl->AssignLayout(rReq, mePageKind);
            }
            Cancel();
			rReq.Done ();
		}
        break;

		case SID_RENAMEPAGE:
		case SID_RENAME_MASTER_PAGE:
        {
			if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
			{
				if ( mpDrawView->IsTextEdit() )
				{
					mpDrawView->SdrEndTextEdit();
				}

                sal_uInt32 nPageId = maTabControl.GetCurPageId();
                SdPage* pCurrentPage = ( GetEditMode() == EM_PAGE )
                    ? GetDoc()->GetSdPage( nPageId - 1, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPageId - 1, GetPageKind() );

                String aTitle( SdResId( STR_TITLE_RENAMESLIDE ) );
                String aDescr( SdResId( STR_DESC_RENAMESLIDE ) );
                String aPageName = pCurrentPage->GetName();

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
				DBG_ASSERT(pFact, "Dialogdiet fail!");
				AbstractSvxNameDialog* aNameDlg = pFact->CreateSvxNameDialog( GetActiveWindow(), aPageName, aDescr );
				DBG_ASSERT(aNameDlg, "Dialogdiet fail!");
				aNameDlg->SetText( aTitle );
                aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ), true );
                aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                if( aNameDlg->Execute() == RET_OK )
                {
                    String aNewName;
                    aNameDlg->GetName( aNewName );
                    if( ! aNewName.Equals( aPageName ) )
                    {
#ifdef DBG_UTIL
                        bool bResult =
#endif
							RenameSlide( nPageId, aNewName );
                        DBG_ASSERT( bResult, "Couldn't rename slide" );
                    }
                }
				delete aNameDlg;
            }

			Cancel();
			rReq.Ignore ();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
		{
			if (mePageKind==PK_STANDARD || mePageKind==PK_NOTES )
			{
				if ( mpDrawView->IsTextEdit() )
				{
					mpDrawView->SdrEndTextEdit();
				}

				maTabControl.StartEditMode( maTabControl.GetCurPageId() );
			}

			Cancel();
			rReq.Ignore ();
		}
		break;

		case SID_PAGESIZE :  // entweder dieses (kein menueeintrag o. ae. !!)
		{
			const SfxItemSet *pArgs = rReq.GetArgs ();

			if (pArgs)
				if (pArgs->Count () == 3)
				{
					SFX_REQUEST_ARG (rReq, pWidth, SfxUInt32Item, ID_VAL_PAGEWIDTH );
					SFX_REQUEST_ARG (rReq, pHeight, SfxUInt32Item, ID_VAL_PAGEHEIGHT );
					SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS );

					const basegfx::B2DVector aSize (pWidth->GetValue (), pHeight->GetValue ());

					SetupPage (aSize, 0, 0, 0, 0, true, false, pScaleAll->GetValue ());
					rReq.Ignore ();
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			rReq.Ignore ();
			break;
		}

		case SID_PAGEMARGIN :  // oder dieses (kein menueeintrag o. ae. !!)
		{
			const SfxItemSet *pArgs = rReq.GetArgs ();

			if (pArgs)
				if (pArgs->Count () == 5)
				{
					SFX_REQUEST_ARG (rReq, pLeft, SfxUInt32Item, ID_VAL_PAGELEFT );
					SFX_REQUEST_ARG (rReq, pRight, SfxUInt32Item, ID_VAL_PAGERIGHT );
					SFX_REQUEST_ARG (rReq, pUpper, SfxUInt32Item, ID_VAL_PAGETOP );
					SFX_REQUEST_ARG (rReq, pLower, SfxUInt32Item, ID_VAL_PAGEBOTTOM );
					SFX_REQUEST_ARG (rReq, pScaleAll, SfxBoolItem, ID_VAL_SCALEOBJECTS );

					const basegfx::B2DVector aEmptySize (0.0, 0.0);

					SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
							   pUpper->GetValue (), pLower->GetValue (),
							   false, true, pScaleAll->GetValue ());
					rReq.Ignore ();
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			rReq.Ignore ();
			break;
		}

		case SID_ATTR_ZOOMSLIDER:
		{
			const SfxItemSet* pArgs = rReq.GetArgs();

			if (pArgs && pArgs->Count () == 1 )
			{
				SFX_REQUEST_ARG (rReq, pScale, SfxUInt16Item, SID_ATTR_ZOOMSLIDER );
				if (CHECK_RANGE (5, pScale->GetValue (), 3000))
				{
					SetZoom (pScale->GetValue ());

					SfxBindings& rBindings = GetViewFrame()->GetBindings();
					rBindings.Invalidate( SID_ATTR_ZOOM );
					rBindings.Invalidate( SID_ZOOM_IN );
					rBindings.Invalidate( SID_ZOOM_OUT );
					rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

				}
			}

			Cancel();
			rReq.Done ();
			break;
		}
		case SID_ZOOMING :	// kein Menueintrag, sondern aus dem Zoomdialog generiert
		{
			const SfxItemSet* pArgs = rReq.GetArgs();

			if (pArgs)
				if (pArgs->Count () == 1)
				{
					SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM );
					if (CHECK_RANGE (10, pScale->GetValue (), 1000))
					{
						SetZoom (pScale->GetValue ());

						SfxBindings& rBindings = GetViewFrame()->GetBindings();
						rBindings.Invalidate( SID_ATTR_ZOOM );
						rBindings.Invalidate( SID_ZOOM_IN );
						rBindings.Invalidate( SID_ZOOM_OUT );
						rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );
					}
					else StarBASIC::FatalError (SbERR_BAD_PROP_VALUE);

					rReq.Ignore ();
					break;
				}

			StarBASIC::FatalError (SbERR_WRONG_ARGS);
			rReq.Ignore ();
			break;
		}

		case SID_ATTR_ZOOM:
		{
			const SfxItemSet* pArgs = rReq.GetArgs();
			mbZoomOnPage = false;

			if ( pArgs )
			{
				SvxZoomType eZT = ( ( const SvxZoomItem& ) pArgs->
											Get( SID_ATTR_ZOOM ) ).GetType();
				switch( eZT )
				{
					case SVX_ZOOM_PERCENT:
						SetZoom( (long) ( ( const SvxZoomItem& ) pArgs->
											Get( SID_ATTR_ZOOM ) ).GetValue() );
						break;

					case SVX_ZOOM_OPTIMAL:
						GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
									SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
						break;

					case SVX_ZOOM_PAGEWIDTH:
						GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
									SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
						break;

					case SVX_ZOOM_WHOLEPAGE:
						GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
									SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );
						break;
					case SVX_ZOOM_PAGEWIDTH_NOBORDER:
						DBG_ERROR("sd::DrawViewShell::FuTemporary(), SVX_ZOOM_PAGEWIDTH_NOBORDER not handled!" );
						break;
				}
				rReq.Ignore ();
			}
			else
			{
				// hier den Zoom-Dialog oeffnen
				SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
			}
			Cancel();
		}
		break;

		case SID_CHANGEBEZIER:
		case SID_CHANGEPOLYGON:
			if ( mpDrawView->IsTextEdit() )
			{
				mpDrawView->SdrEndTextEdit();
				GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
			}

			if ( mpDrawView->IsPresObjSelected() )
			{
                ::sd::Window* pWindow = GetActiveWindow();
				InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
			}
			else
			{
				if( rReq.GetSlot() == SID_CHANGEBEZIER )
				{
					WaitObject aWait( (Window*)GetActiveWindow() );
					mpDrawView->ConvertMarkedToPathObj(false);
				}
				else
				{
					if( mpDrawView->IsVectorizeAllowed() )
                    {
						SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView, GetDoc(), rReq ) );
                    }
					else
					{
						WaitObject aWait( (Window*)GetActiveWindow() );
						mpDrawView->ConvertMarkedToPolyObj(false);
					}
				}

				Invalidate(SID_CHANGEBEZIER);
				Invalidate(SID_CHANGEPOLYGON);
			}
			Cancel();

			if( HasCurrentFunction(SID_BEZIER_EDIT) )
			{	// ggf. die richtige Editfunktion aktivieren
				GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
										SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
			}
			rReq.Ignore ();
			break;

		case SID_CONVERT_TO_CONTOUR:
			if ( mpDrawView->IsTextEdit() )
			{
				mpDrawView->SdrEndTextEdit();
				GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
			}

			if ( mpDrawView->IsPresObjSelected() )
			{
                ::sd::Window* pWindow = GetActiveWindow();
				InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
			}
			else
			{
				WaitObject aWait( (Window*)GetActiveWindow() );
				mpDrawView->ConvertMarkedToPathObj(true);

				Invalidate(SID_CONVERT_TO_CONTOUR);
			}
			Cancel();

			rReq.Ignore ();
			break;

		case SID_CONVERT_TO_METAFILE:
		case SID_CONVERT_TO_BITMAP:
		{
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( mpDrawView->IsTextEdit() )
			{
				mpDrawView->SdrEndTextEdit();
				GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);
            }

			if ( mpDrawView->IsPresObjSelected(true,true,true) )
			{
                ::sd::Window* pWindow = GetActiveWindow();
				InfoBox(pWindow, String(SdResId(STR_ACTION_NOTPOSSIBLE) ) ).Execute();
			}
			else
			{
				WaitObject aWait( (Window*)GetActiveWindow() );

				// switch on undo for the next operations
				mpDrawView->BegUndo(
					String(
					SdResId (nSId==SID_CONVERT_TO_METAFILE ? STR_UNDO_CONVERT_TO_METAFILE : STR_UNDO_CONVERT_TO_BITMAP)));

				// create SdrGrafObj from metafile/bitmap
				Graphic aGraphic;
				switch (nSId)
				{
					case SID_CONVERT_TO_METAFILE:
					{
						GDIMetaFile aMetaFile(mpDrawView->GetMarkedObjMetaFile());
						aGraphic = Graphic(aMetaFile);
					}
					break;
					case SID_CONVERT_TO_BITMAP:
					{
                        bool bDone(false);
                        
                        // aw080: Take a look at AAed graphic creation
						// I have to get the image here directly since GetMarkedObjBitmapEx works
                        // based on Bitmaps, but not on BitmapEx, thus throwing away the alpha
                        // channel. Argh! GetMarkedObjBitmapEx itself is too widely used to safely
                        // change that, e.g. in the exchange formats. For now I can only add this 
                        // exception to get good results for Svgs. This is how the code gets more 
                        // and more crowded, at last I made a remark for myself to change this
                        // as one of the next tasks.
                        const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(mpDrawView->getSelectedIfSingle());

                        if(pSdrGrafObj && pSdrGrafObj->isEmbeddedSvg())
                        {
                            aGraphic = Graphic(pSdrGrafObj->GetGraphic().getSvgData()->getReplacement());
                            bDone = true;
                        }

                        if(!bDone)
                        {
                            aGraphic = Graphic(mpDrawView->GetMarkedObjBitmapEx());
                        }
					}
					break;
				}

				// create new object
				SdrGrafObj* pGraphicObj = new SdrGrafObj(
					*GetDoc(),
					aGraphic);

				// get some necessary info and ensure it
				const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
				OSL_ENSURE(aSelection.size(), "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP with empty selection (!)");

				// fit rectangle of new graphic object to selection's mark rect
				const basegfx::B2DRange aAllMarkedRange(sdr::legacy::GetAllObjBoundRange(aSelection));
				sdr::legacy::SetLogicRange(*pGraphicObj, aAllMarkedRange);

				// #i71540# to keep the order, it is necessary to replace the lowest object
				// of the selection with the new object. This also means that with multi
				// selection, all other objects need to be deleted first
				SdrObject* pReplacementCandidate = aSelection[0];

				if(aSelection.size() > 1)
				{
					// take first object out of selection
					mpDrawView->MarkObj(*pReplacementCandidate, true );

					// clear remaining selection
					mpDrawView->DeleteMarkedObj();
				}

				// now replace lowest object with new one
				mpDrawView->ReplaceObjectAtView(*pReplacementCandidate, *pGraphicObj);

				// switch off undo
				mpDrawView->EndUndo();
			}
		}

		Cancel();

		rReq.Done ();
		break;

		case SID_SET_DEFAULT:
		{
			SfxItemSet* pSet = NULL;

			if (mpDrawView->IsTextEdit())
			{
				::Outliner* pOutl = mpDrawView->GetTextEditOutliner();
				if (pOutl)
				{
					pOutl->RemoveFields(true, &typeid(SvxURLField));
				}

				pSet = new SfxItemSet( GetPool(), EE_ITEMS_START, EE_ITEMS_END );
				mpDrawView->SetAttributes( *pSet, true );
			}
			else
			{
				if(mpDrawView->GetSdrPageView())
				{
    				// In diese Liste werden fuer jedes Praesentationsobjekt ein SfxItemSet
					// der harten Attribute sowie die connection zum SdrObject eingetragen, da diese beim nachfolgenden
					// mpDrawView->SetAttributes( *pSet, true ) verloren gehen und spaeter restauriert
	    			// werden muessen
					const SdrObjectVector aSelection(mpDrawView->getSelectedSdrObjectVectorFromSdrMarkView());
					::std::vector< SfxItemSet* > aAttrVector;
					::std::vector< SdrPage* > aPageVector;
					SdPage& rPresPage = (SdPage&) mpDrawView->GetSdrPageView()->getSdrPageFromSdrPageView();
					sal_uInt32 i;

					for ( i = 0; i < aSelection.size(); i++ )
		    		{
						SdrObject* pObj = aSelection[i];

						if( rPresPage.IsPresObj( pObj ) )
			    		{
							SfxItemSet* pNewSet = new SfxItemSet( GetDoc()->GetItemPool(), SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT, 0 );
    						pNewSet->Put(pObj->GetMergedItemSet());
							aAttrVector.push_back( pNewSet );
							aPageVector.push_back( findConnectionToSdrObject(pObj) );
					    }
				    }

    				pSet = new SfxItemSet( GetPool() );
					mpDrawView->SetAttributes( *pSet, true );
					sal_uInt32 j = 0;

					for ( i = 0; i < aSelection.size(); i++ )
	    			{
		    			SfxStyleSheet* pSheet = NULL;
						SdrObject* pObj = aSelection[i];

					    if (pObj->GetObjIdentifier() == OBJ_TITLETEXT)
					    {
						    pSheet = mpActualPage->GetStyleSheetForPresObj(PRESOBJ_TITLE);
						    if (pSheet)
								pObj->SetStyleSheet(pSheet, false);
					    }
					    else if(pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
					    {
						    for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
						    {
							    pSheet = mpActualPage->GetStyleSheetForPresObj( PRESOBJ_OUTLINE );
							    DBG_ASSERT(pSheet, "Vorlage fuer Gliederungsobjekt nicht gefunden");
							    if (pSheet)
							    {
								    pObj->StartListening(*pSheet);

								    if( nLevel == 1 )
									// Textrahmen hoert auf StyleSheet der Ebene1
										pObj->SetStyleSheet(pSheet, false);
							    }
						    }
					    }

						if( rPresPage.IsPresObj( pObj ) )
    					{
							SfxItemSet* pNewSet = aAttrVector[j];

						    if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SFX_ITEM_ON )
						    {
							    pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
						    }

						    if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SFX_ITEM_ON )
						    {
							    pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
						    }

							SdPage* pConnectionToSdrObject = dynamic_cast< SdPage* >(aPageVector[j++]);

							if( pConnectionToSdrObject )
							{
								establishConnectionToSdrObject(pObj, pConnectionToSdrObject);
							}

						    delete pNewSet;
					    }
				    }
				}
			}

			delete pSet;
			Cancel();
		}
		break;

		case SID_DELETE_SNAPITEM:
		{
			const basegfx::B2DPoint aMPos(GetActiveWindow()->GetInverseViewTransformation() * maMousePos);
			const double fHitLog(basegfx::B2DVector(GetActiveWindow()->GetInverseViewTransformation() * basegfx::B2DVector(FuPoor::HITPIX, 0.0)).getLength());
			sal_uInt32 nHelpLine;
			mbMousePosFreezed = false;

			if( mpDrawView->PickHelpLine( aMPos, fHitLog, nHelpLine) )
			{
				if(mpDrawView->GetSdrPageView())
    			{
					mpDrawView->GetSdrPageView()->DeleteHelpLine( nHelpLine );
	    		}
			}

			Cancel();
			rReq.Ignore ();
		}
		break;

		case SID_DELETE_PAGE:
		case SID_DELETE_MASTER_PAGE:
			DeleteActualPage();
			Cancel();
			rReq.Ignore ();
		break;

		case SID_DELETE_LAYER:
			DeleteActualLayer();
			Cancel();
			rReq.Ignore ();
		break;

		case SID_ORIGINAL_SIZE:
			mpDrawView->SetMarkedOriginalSize();
			Cancel();
			rReq.Done();
		break;

		case SID_DRAW_FONTWORK:
		case SID_DRAW_FONTWORK_VERTICAL:
		{
			svx::FontworkBar::execute( mpView, rReq, GetViewFrame()->GetBindings() );		// SJ: can be removed  (I think)
			Cancel();
			rReq.Done();
		}
		break;

		case SID_SAVEGRAPHIC:
		{
			SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >(mpDrawView->getSelectedIfSingle());
			
			if(pGrafObj )
			{
				::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape( pGrafObj->getUnoShape(), com::sun::star::uno::UNO_QUERY );
				SdGRFFilter::SaveGraphic( xShape );
			}
			
			Cancel();
			rReq.Ignore();
		}
		break;

		default:
		{
			// switch Anweisung wegen CLOOKS aufgeteilt. Alle case-Anweisungen die
			// eine Fu???? -Funktion aufrufen, sind in die Methode FuTemp01 (drviews8)
			// gewandert.
			FuTemp01(rReq);
		}
		break;
	}

	if(HasCurrentFunction())
	{
		GetCurrentFunction()->Activate();
	}
}




/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    SdPage* pNewPage = NULL;
    if (ePageKind == PK_STANDARD && meEditMode != EM_MASTERPAGE)
    {
        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }
        pNewPage = ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage, nInsertPosition);
    }
    return pNewPage;
}

} // end of namespace sd
