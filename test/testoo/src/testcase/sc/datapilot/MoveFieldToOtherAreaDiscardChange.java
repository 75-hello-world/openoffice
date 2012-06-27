/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

/**
 * 
 */
package testcase.sc.datapilot;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.MenuItem;
import java.awt.Rectangle;
import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;


/**
 *
 *
 */
public class MoveFieldToOtherAreaDiscardChange {
	/**
	 * TestCapture helps us to do 1. Take a screenshot when failure occurs. 2.
	 * Collect extra data when OpenOffice crashes.
	 */
	@Rule
	public Log LOG = new Log();

	/**
	 * initApp helps us to do 1. Patch the OpenOffice to enable automation if
	 * necessary. 2. Start OpenOffice with automation enabled if necessary. 3.
	 * Reset OpenOffice to startcenter.
	 * 
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		initApp();
	}
	/**
	 * 
	 * Verify that DP panel will be synchronized with table while move fields on panel.
	 */
	@Test
	public void test() {
		String file = testFile("source_data01.ods");
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		sleep(2);
		CalcUtil.selectRange("A1:E27");
		calc.menuItem("Data->DataPilot->Start...").select();
		CreateDataPilotTableDialog.ok();
		if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
			DataPilotAutomaticallyUpdateCheckBox.check();
			
		}
		assertEquals("New DataPilot Table",CalcUtil.getCellText("B2"));
		assertEquals("Use the DataPilot panel to assign fields to areas in the DataPilot table.",CalcUtil.getCellText("B4"));
		assertEquals("The DataPilot panel automatically displays when the DataPilot table has focus.",CalcUtil.getCellText("B5"));
		assertEquals("Page Area",CalcUtil.getCellText("B7"));
		assertEquals("Row Area",CalcUtil.getCellText("B10"));
		assertEquals("Column Area",CalcUtil.getCellText("D9"));
		assertEquals("Data Area",CalcUtil.getCellText("D11"));
		sleep(1);
		
		DataPilotFieldSelect.click(1, 1);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Column").select();
		assertEquals("Locale",CalcUtil.getCellText("A1"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,30);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Column").select();
		assertEquals("Name",CalcUtil.getCellText("B1"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,50);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Row").select();
		assertEquals("Date",CalcUtil.getCellText("A3"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,70);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Row").select();
		assertEquals("Order Number",CalcUtil.getCellText("B3"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,90);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Data by->Sum").select();
		assertEquals("Sum - Amount",CalcUtil.getCellText("A1"));
		assertEquals("32779.17",CalcUtil.getCellInput("K30"));
		sleep(1);
		
		DataPilotAutomaticallyUpdateCheckBox.uncheck();
		
		DataPilotPaneRowArea.drag(1,1, 1, -50);
		assertEquals("Date",CalcUtil.getCellText("A3"));
		assertEquals("32779.17",CalcUtil.getCellInput("K30"));
		sleep(1);
		
		CalcUtil.selectRange("B33");
		ActiveMsgBox.yes();
		assertEquals("Date",CalcUtil.getCellText("D1"));
		assertEquals("32779.17",CalcUtil.getCellInput("AB31"));
		sleep(1);
		
		DataPilotColumnArea.click(1,50);
		DataPilotColumnArea.openContextMenu();
		menuItem("Move to Row").select();
		assertEquals("Date",CalcUtil.getCellText("D1"));
		assertEquals("32779.17",CalcUtil.getCellInput("AB31"));
		
		CalcUtil.selectRange("B33");
		ActiveMsgBox.yes();
		assertEquals("Date",CalcUtil.getCellText("B3"));
		assertEquals("32779.17",CalcUtil.getCellInput("K30"));
		sleep(1);
		
		DataPilotAutomaticallyUpdateCheckBox.check();
	}

}
