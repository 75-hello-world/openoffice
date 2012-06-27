/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package testcase.sc.validity;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.vcl.IDList;
import org.openoffice.test.vcl.widgets.VclMessageBox;


import testlib.CalcUtil;
import testlib.Log;


public class AllowDecimalEqual {
	private static IDList idList = new IDList(new File("./ids"));
	public static final VclMessageBox ActiveMsgBox = new VclMessageBox(idList.getId("UID_ACTIVE"), "Message on message box.");
	
	/**
	 * TestCapture helps us to do
	 * 1. Take a screenshot when failure occurs.
	 * 2. Collect extra data when OpenOffice crashes.
	 */	
	@Rule
	public Log LOG = new Log();
	
	/**
	 * initApp helps us to do 
	 * 1. Patch the OpenOffice to enable automation if necessary.
	 * 2. Start OpenOffice with automation enabled if necessary.
	 * 3. Reset OpenOffice to startcenter.
	 * 
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		initApp();
	}
	
	/**
	 * test Allow Decimal equal in Validity.
	 */
	@Test
	public void testAllowDecimalEqual() {
		startcenter.menuItem("File->New->Spreadsheet").select();
		sleep(1);

		CalcUtil.selectRange("Sheet1.A1:B5");
		calc.menuItem("Data->Validity...").select();
		sleep(1);
		
		SC_ValidityCriteriaTabpage.select();
		SC_ValidityCriteriaAllowList.select("Decimal");
		SC_ValidityDecimalCompareOperator.select("equal");
		SC_ValiditySourceInput.setText("0.33333333");
		SC_ValidityErrorAlertTabPage.select();
		SC_ValidityShowErrorMessage.check();
		SC_ValidityErrorMessageTitle.setText("Stop to enter");
		SC_ValidityErrorMessage.setText("Invalid value");
		typeKeys("<tab>");
		typeKeys("<enter>");	
		sleep(1);
				
		CalcUtil.selectRange("Sheet1.A1");
		SC_CellInput.activate();
		typeKeys("0.33333333");
		typeKeys("<enter>");
		assertEquals("0.33333333",CalcUtil.getCellText("Sheet1.A1"));
		
		CalcUtil.selectRange("Sheet1.A2");
		SC_CellInput.activate();
		typeKeys("=1/3");
		typeKeys("<enter>");
		assertEquals("0.33333333",CalcUtil.getCellText("Sheet1.A2"));
		
		CalcUtil.selectRange("Sheet1.A3");
		SC_CellInput.activate();
		typeKeys("0.3");
		typeKeys("<enter>");
		assertEquals("Invalid value",ActiveMsgBox.getMessage());
		ActiveMsgBox.ok();
		assertEquals("",CalcUtil.getCellText("Sheet1.A3"));

		CalcUtil.selectRange("Sheet1.A4");
		SC_CellInput.activate();
		typeKeys("0.333333333");
		typeKeys("<enter>");
		assertEquals("Invalid value",ActiveMsgBox.getMessage());
		ActiveMsgBox.ok();
		assertEquals("",CalcUtil.getCellText("Sheet1.A4"));

		CalcUtil.selectRange("Sheet1.B2");
		SC_CellInput.activate();
		typeKeys("test");
		typeKeys("<enter>");
		assertEquals("Invalid value",ActiveMsgBox.getMessage());
		ActiveMsgBox.ok();
		assertEquals("",CalcUtil.getCellText("Sheet1.B2"));

		}

}

