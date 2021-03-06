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



// __________ Imports __________

// base classes
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.*;

// property access
import com.sun.star.beans.*;

// application specific classes
import com.sun.star.chart.*;
import com.sun.star.drawing.*;
import com.sun.star.text.XTextDocument;

import com.sun.star.table.CellRangeAddress;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XController;

import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.util.XNumberFormats;

// base graphics things
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.FontWeight;
import com.sun.star.awt.FontRelief;

// Exceptions
import com.sun.star.uno.Exception;
import com.sun.star.uno.RuntimeException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.util.MalformedNumberFormatException;


// __________ Implementation __________

/** Test to create a writer document and insert an OLE Chart.

    Be careful!  This does not really work.  The Writer currently has no
    interface for dealing with OLE objects.  You can add an OLE shape to the
    Writer's drawing layer, but it is not treated correctly as OLE object.
    Thus, you can not activate the chart by double-clicking.  The office may
    also crash when the document is closed!

    @author Bj&ouml;rn Milcke
 */
public class ChartInWriter
{
    // ____________________

    public static void main( String args[] )
    {
        Helper aHelper = new Helper( args );

        ChartHelper aChartHelper = new ChartHelper(
            (XModel) UnoRuntime.queryInterface( XModel.class,
                                                aHelper.createTextDocument()));

        // the unit for measures is 1/100th of a millimeter
        // position at (1cm, 1cm)
        Point aPos    = new Point( 1000, 1000 );

        // size of the chart is 15cm x 12cm
        Size  aExtent = new Size( 15000, 13000 );

        // insert a new chart into the "Chart" sheet of the
        // spreadsheet document
        XChartDocument aChartDoc = aChartHelper.insertOLEChartInWriter(
            "BarChart",
            aPos,
            aExtent,
            "com.sun.star.chart.AreaDiagram" );

        // instantiate test class with newly created chart
        ChartInWriter aTest   = new ChartInWriter( aChartDoc );

        try
        {
             aTest.lockControllers();

            // do tests here
             aTest.testWall();

             aTest.unlockControllers();
        }
        catch( Exception ex )
        {
            System.out.println( "UNO Exception caught: " + ex );
            System.out.println( "Message: " + ex.getMessage() );
        }

        System.exit( 0 );
    }


    // ________________________________________

    public ChartInWriter( XChartDocument aChartDoc )
    {
        maChartDocument = aChartDoc;
        maDiagram       = maChartDocument.getDiagram();
    }

    // ____________________

    public void lockControllers()
        throws RuntimeException
    {
        ((XModel) UnoRuntime.queryInterface( XModel.class, maChartDocument )).lockControllers();
    }

    // ____________________

    public void unlockControllers()
        throws RuntimeException
    {
        ((XModel) UnoRuntime.queryInterface( XModel.class, maChartDocument )).unlockControllers();        
    }

    // ____________________

    public void testWall()
        throws RuntimeException, UnknownPropertyException, PropertyVetoException,
               com.sun.star.lang.IllegalArgumentException, WrappedTargetException
    {
        XPropertySet aWall = ((X3DDisplay) UnoRuntime.queryInterface(
                                  X3DDisplay.class, maDiagram )).getWall();

        // change background color of area
        aWall.setPropertyValue( "FillColor", new Integer( 0xeecc99 ));
        aWall.setPropertyValue( "FillStyle",  FillStyle.SOLID );
    }

    // ______________________________
    //
    // private members
    // ______________________________

    private XChartDocument maChartDocument;
    private XDiagram       maDiagram;
}
