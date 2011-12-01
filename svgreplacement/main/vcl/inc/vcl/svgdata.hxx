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
#ifndef _SV_SVGDATA_HXX
#define _SV_SVGDATA_HXX

#include <basegfx/range/b2drange.hxx>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////

typedef boost::shared_array< sal_uInt8 > SvgDataArray;
typedef ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > Primitive2DReference;
typedef ::com::sun::star::uno::Sequence< Primitive2DReference > Primitive2DSequence;

//////////////////////////////////////////////////////////////////////////////

class VCL_DLLPUBLIC SvgData : private boost::noncopyable
{
private:
    // the file and length
    SvgDataArray            maSvgDataArray;
    sal_uInt32              mnSvgDataArrayLength;

    // on demand created content
    basegfx::B2DRange       maRange;
    Primitive2DSequence     maSequence;
    BitmapEx                maReplacement;

    // on demand creators
    void ensureReplacement();
    void ensureSequenceAndRange();

public:
    SvgData(const SvgDataArray& rSvgDataArray, sal_uInt32 nSvgDataArrayLength);

    /// data read
    const SvgDataArray& getSvgDataArray() const { return maSvgDataArray; }
    sal_uInt32 getSvgDataArrayLength() const { return mnSvgDataArrayLength; }

    /// data read and evtl. on demand creation
    const basegfx::B2DRange& getRange() const;
    const Primitive2DSequence& getPrimitive2DSequence() const;
    const BitmapEx& getReplacement() const;
};

//////////////////////////////////////////////////////////////////////////////

typedef boost::shared_ptr< SvgData > SvgDataPtr;

//////////////////////////////////////////////////////////////////////////////

#endif // _SV_SVGDATA_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
