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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGSTYLENODE_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGSTYLENODE_HXX

#include <svgio/svgiodllapi.h>
#include <svgio/svgreader/svgnode.hxx>
#include <svgio/svgreader/svgstyleattributes.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        class SvgStyleNode : public SvgNode
        {
        private:
            /// use styles
            std::vector< SvgStyleAttributes* >      maSvgStyleAttributes;

            /// bitfield
            bool                                    mbTextCss : 1; // true == type is 'text/css'

        public:
            SvgStyleNode(
                SvgDocument& rDocument,
                SvgNode* pParent);
            virtual ~SvgStyleNode();

            virtual void parseAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);
            void addCssStyleSheet(const rtl::OUString& aContent);

            /// textCss access
            bool isTextCss() const { return mbTextCss; }
            void setTextCss(bool bNew) { mbTextCss = bNew; }
        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGSTYLENODE_HXX

// eof
