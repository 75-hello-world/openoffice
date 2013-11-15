/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright IBM Corporation 2010.
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

//IAccessibility2 Implementation 2009-----
#ifndef _SVTRULERACCESSIBLE_HXX
#define _SVTRULERACCESSIBLE_HXX

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ILLEGALACCESSIBLECOMPONENTSTATEEXCEPTION_HPP_
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef	_COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef	_CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICENAME_HPP_
#include <com/sun/star/lang/XServiceName.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
#ifndef _UTL_SERVICEHELPER_HXX_
#include <comphelper/servicehelper.hxx>
#endif

namespace com { namespace sun { namespace star { namespace awt {
	struct Point;
	struct Rectangle;
	struct Size;
	class XFocusListener;
} } } }

class Rectangle;
class Ruler;
class Window;


typedef ::cppu::WeakAggComponentImplHelper5<
			::com::sun::star::accessibility::XAccessible,
			::com::sun::star::accessibility::XAccessibleComponent,
			::com::sun::star::accessibility::XAccessibleContext,
			::com::sun::star::accessibility::XAccessibleEventBroadcaster,
			::com::sun::star::lang::XServiceInfo >
			SvtRulerAccessible_Base;

class SvtRulerAccessible : public ::comphelper::OBaseMutex, public SvtRulerAccessible_Base
{
public:
	//=====  internal  ========================================================
	SvtRulerAccessible(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent, Ruler&	rRepresentation, const ::rtl::OUString& rName );
protected:
	virtual ~SvtRulerAccessible();
public:
	//=====  XAccessible  =====================================================

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
		getAccessibleContext( void ) throw( ::com::sun::star::uno::RuntimeException );

	//=====  XAccessibleComponent  ============================================

	virtual sal_Bool SAL_CALL
		containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
		getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::awt::Rectangle SAL_CALL
		getBounds() throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::awt::Point SAL_CALL
		getLocation() throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::awt::Point SAL_CALL
		getLocationOnScreen() throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::awt::Size SAL_CALL
		getSize() throw( ::com::sun::star::uno::RuntimeException );

	virtual sal_Bool SAL_CALL
		isShowing() throw( ::com::sun::star::uno::RuntimeException );

	virtual sal_Bool SAL_CALL
		isVisible() throw( ::com::sun::star::uno::RuntimeException );

	virtual sal_Bool SAL_CALL
		isFocusTraversable() throw( ::com::sun::star::uno::RuntimeException );

	virtual void SAL_CALL
		addFocusListener(
			const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
			throw( ::com::sun::star::uno::RuntimeException );

	virtual void SAL_CALL
		removeFocusListener(
			const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener )
			throw( ::com::sun::star::uno::RuntimeException );

	virtual void SAL_CALL
		grabFocus() throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Any SAL_CALL
		getAccessibleKeyBinding() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL 
        getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL 
        getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

	//=====  XAccessibleContext  ==============================================

	virtual sal_Int32 SAL_CALL
		getAccessibleChildCount( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
		getAccessibleChild( sal_Int32 nIndex )
			throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
		getAccessibleParent( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual	sal_Int32 SAL_CALL
		getAccessibleIndexInParent( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual sal_Int16 SAL_CALL
		getAccessibleRole( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::rtl::OUString SAL_CALL
		getAccessibleDescription( void ) throw (::com::sun::star::uno::RuntimeException);

	virtual ::rtl::OUString SAL_CALL
		getAccessibleName( void ) throw (::com::sun::star::uno::RuntimeException);

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
		getAccessibleRelationSet( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
		getAccessibleStateSet( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::lang::Locale SAL_CALL
		getLocale( void )
			throw(	::com::sun::star::uno::RuntimeException,
					::com::sun::star::accessibility::IllegalAccessibleComponentStateException );
	//=====  XAccessibleEventBroadcaster  =====================================

	virtual void SAL_CALL
		addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )throw( com::sun::star::uno::RuntimeException );

	virtual void SAL_CALL 
		removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );

	// Needed, because some compilers would complain about virtual functions above would hide cppu::WeakAggComponentImplHelperBase::add/removeEventListener(const Reference<lang::XEventListener>&)...
	virtual void SAL_CALL
		addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )throw( com::sun::star::uno::RuntimeException );

	virtual void SAL_CALL 
		removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException );


	//=====  XServiceInfo  ====================================================

	virtual ::rtl::OUString SAL_CALL
		getImplementationName( void ) throw( ::com::sun::star::uno::RuntimeException );

	virtual sal_Bool SAL_CALL
		supportsService( const ::rtl::OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException );

	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
		getSupportedServiceNames( void ) throw( ::com::sun::star::uno::RuntimeException );

	//=====  XTypeProvider  ===================================================

	virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
		getImplementationId( void ) throw( ::com::sun::star::uno::RuntimeException );




public:

	/// Sets the name
	void setName( const ::rtl::OUString& rName );

	/// Sets the description
	void setDescription( const ::rtl::OUString& rDescr );
private:
	static ::com::sun::star::uno::Sequence< sal_Int8 > getUniqueId( void );
protected:

    ///	@Return the object's current bounding box relative to the desktop.
	virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::com::sun::star::uno::RuntimeException );

	///	@Return the object's current bounding box relative to the parent object.
	virtual Rectangle GetBoundingBox( void ) throw( ::com::sun::star::uno::RuntimeException );


	virtual void SAL_CALL disposing();

	/// @returns true if it's disposed or in disposing
	inline sal_Bool IsAlive( void ) const;

	/// @returns true if it's not disposed and no in disposing
	inline sal_Bool IsNotAlive( void ) const;

	/// throws the exception DisposedException if it's not alive
	void ThrowExceptionIfNotAlive( void ) throw( ::com::sun::star::lang::DisposedException );

private:
    /**	Description of this object.  This is not a constant because it can
    	be set from the outside.
    */
    ::rtl::OUString						msDescription;

    /**	Name of this object.
    */
    ::rtl::OUString						msName;

    ///	Reference to the parent object.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
										mxParent;

	/// pointer to internal representation
	Ruler*							mpRepr;

	    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;


};

inline sal_Bool SvtRulerAccessible::IsAlive( void ) const
{
	return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline sal_Bool SvtRulerAccessible::IsNotAlive( void ) const
{
	return rBHelper.bDisposed || rBHelper.bInDispose;
}



#endif
//-----IAccessibility2 Implementation 2009

