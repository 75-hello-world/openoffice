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

#ifndef SFX_SIDEBAR_CONTROLLER_HXX
#define SFX_SIDEBAR_CONTROLLER_HXX

#include "ResourceManager.hxx"
#include "AsynchronousCall.hxx"
#include "TabBar.hxx"
#include "Context.hxx"
#include "Panel.hxx"

#include <vcl/menu.hxx>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <boost/noncopyable.hpp>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;


namespace
{
    typedef ::cppu::WeakComponentImplHelper3 <
        css::ui::XContextChangeEventListener,
        css::beans::XPropertyChangeListener,
        css::ui::XSidebar
        > SidebarControllerInterfaceBase;
}

namespace sfx2 { namespace sidebar {

class ContentPanelDescriptor;
class Deck;
class DeckDescriptor;
class SidebarDockingWindow;
class TabBar;
class TabBarConfiguration;

class SidebarController
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public SidebarControllerInterfaceBase
{
public:
    SidebarController(
        SidebarDockingWindow* pParentWindow,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    virtual ~SidebarController (void);

    // ui::XContextChangeEventListener
    virtual void SAL_CALL notifyContextChangeEvent (const css::ui::ContextChangeEventObject& rEvent)
        throw(cssu::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEventObject)
        throw(cssu::RuntimeException);

    // beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange (const css::beans::PropertyChangeEvent& rEvent)
        throw(cssu::RuntimeException);

    // ui::XSidebar
    virtual void SAL_CALL requestLayout (void)
        throw(cssu::RuntimeException);
    
    void NotifyResize (void);

    void SwitchToDeck (
        const ::rtl::OUString& rsDeckId);

    /** Show only the tab bar, not the deck.
    */
    void CloseDeck (void);

    /** Open the deck area and restore the parent window to its old width.
    */
    void OpenDeck (void);

private:
    ::boost::scoped_ptr<Deck> mpCurrentDeck;
    SidebarDockingWindow* mpParentWindow;
    ::boost::scoped_ptr<TabBar> mpTabBar;
    cssu::Reference<css::frame::XFrame> mxFrame;
    Context maCurrentContext;
    ::rtl::OUString msCurrentDeckId;
    AsynchronousCall maPropertyChangeForwarder;
    bool mbIsDeckClosed;
    /** Before the deck is closed the sidebar width is saved into this variable,
        so that it can be restored when the deck is reopended.
    */
    sal_Int32 mnSavedSidebarWidth;
    
    DECL_LINK(WindowEventHandler, VclWindowEvent*);
    void UpdateConfigurations (const Context& rContext);
    bool ArePanelSetsEqual (
        const SharedPanelContainer& rCurrentPanels,
        const ResourceManager::PanelContextDescriptorContainer& rRequestedPanels);
    cssu::Reference<css::ui::XUIElement> CreateUIElement (
        const cssu::Reference<css::awt::XWindowPeer>& rxWindow,
        const ::rtl::OUString& rsImplementationURL);
    SharedPanel CreatePanel (
        const ::rtl::OUString& rsPanelId,
        ::Window* pParentWindow,
        const ::rtl::OUString& rsMenuCommand);
    void SwitchToDeck (
        const DeckDescriptor& rDeckDescriptor,
        const Context& rContext);
    void ShowPopupMenu (
        const Rectangle& rButtonBox,
        const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
        const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const;
    void ShowDetailMenu (const ::rtl::OUString& rsMenuCommand) const;
    ::boost::shared_ptr<PopupMenu> CreatePopupMenu (
        const ::std::vector<TabBar::DeckMenuData>& rDeckSelectionData,
        const ::std::vector<TabBar::DeckMenuData>& rDeckShowData) const;
    DECL_LINK(OnMenuItemSelected, Menu*);
    void BroadcastPropertyChange (void);

    /** The close of the deck changes the width of the child window.
        That is only possible if there is no other docking window docked above or below the sidebar.
        Return whether the width of the child window can be modified.
    */
    bool CanModifyChildWindowWidth (void) const;
        
    /** Set the child window container to a new width.
        Return the old width.
    */
    sal_Int32 SetChildWindowWidth (const sal_Int32 nNewWidth);

    void RestrictWidth (void);
    
    virtual void SAL_CALL disposing (void);
};


} } // end of namespace sfx2::sidebar

#endif
