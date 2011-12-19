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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGNODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGNODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgtools.hxx>
#include <svgio/svgreader/svgtoken.hxx>
#include <svgio/svgreader/svgpaint.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <vector>
#include <hash_map>

//////////////////////////////////////////////////////////////////////////////
// predefines
namespace svgio 
{ 
    namespace svgreader 
    { 
        class SvgNode; 
        class SvgDocument; 
        class SvgStyleAttributes;
    }
}

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        typedef ::std::vector< SvgNode* > SvgNodeVector;

        enum XmlSpace
        {
            XmlSpace_notset,
            XmlSpace_default,
            XmlSpace_preserve
        };

        class SvgNode : private boost::noncopyable, public InfoProvider
        {
        private:
            /// basic data, Type, document we belong to and parent (if not root)
            SVGToken                    maType;
            SvgDocument&                mrDocument;
            const SvgNode*              mpParent;
            const SvgNode*              mpAlternativeParent;

            /// sub hierarchy
            SvgNodeVector               maChildren;

            /// Id svan value
            rtl::OUString*              mpId;

            /// Class svan value
            rtl::OUString*              mpClass;

            /// XmlSpace value
            XmlSpace                    maXmlSpace;

        public:
            SvgNode(
                SVGToken aType,
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgNode();

            void parseAttributes(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttribs);
            virtual const SvgStyleAttributes* getSvgStyleAttributes() const;
            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            virtual void decomposeSvgNode(drawinglayer::primitive2d::Primitive2DSequence& rTarget, bool bReferenced) const;

            /// basic data read access
            const SVGToken getType() const { return maType; }
            const SvgDocument& getDocument() const { return mrDocument; }
            const SvgNode* getParent() const { if(mpAlternativeParent) return mpAlternativeParent; return mpParent; }
            const SvgNodeVector& getChildren() const { return maChildren; }

            /// InfoProvider support for %, em and ex values
            virtual const basegfx::B2DRange* getCurrentViewPort() const;
            virtual double getCurrentFontSize() const;
            virtual double getCurrentXHeight() const;

            /// Id access
            const rtl::OUString* getId() const { return mpId; }
            void setId(const rtl::OUString* pfId = 0);

            /// Class access
            const rtl::OUString* getClass() const { return mpClass; }
            void setClass(const rtl::OUString* pfClass = 0);

            /// XmlSpace access
            XmlSpace getXmlSpace() const;
            void setXmlSpace(XmlSpace eXmlSpace = XmlSpace_notset) { maXmlSpace = eXmlSpace; }

            /// alternative parent
            void setAlternativeParent(const SvgNode* pAlternativeParent = 0) { mpAlternativeParent = pAlternativeParent; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGNODE_HXX

// eof
