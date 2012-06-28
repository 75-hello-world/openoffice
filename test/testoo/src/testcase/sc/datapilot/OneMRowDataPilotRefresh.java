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



/**
 * 
 */
package testcase.sc.datapilot;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

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
public class OneMRowDataPilotRefresh {
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
	 * Verify the data pilot result table refresh after source data changed
	 */
	@Test
	public void test() {
		String file = testFile("source_data01.ods");
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		sleep(2);
		calc.maximize();
		CalcUtil.selectRange("A1:E27");
		typeKeys("<$copy>");
		CalcUtil.selectRange("A1048540");
		typeKeys("<$paste>");
		calc.menuItem("Data->DataPilot->Start...").select();
		DataPilotTableToExistPlaceRadioButton.check();
		assertTrue(DataPilotTableToExistPlaceRadioButton.isChecked());
		DataPilotTableToExistPlaceEditBox.setText("$A.$F$1048540");
		CreateDataPilotTableDialog.ok();
		sleep(1);
		if(DataPilotAutomaticallyUpdateCheckBox.isChecked()==false){
			DataPilotAutomaticallyUpdateCheckBox.check();
			
		}
		DataPilotFieldSelect.click(1, 1);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Page").select();
		assertEquals("Locale",CalcUtil.getCellText("F1048540"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,30);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Column").select();
		assertEquals("Name",CalcUtil.getCellText("F1048542"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,50);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Row").select();
		assertEquals("Date",CalcUtil.getCellText("F1048543"));
		sleep(1);
		
		DataPilotFieldSelect.click(1,70);
		DataPilotFieldSelect.openContextMenu();
		menuItem("Add to Data by->Sum").select();
		assertEquals("Sum - Order Number",CalcUtil.getCellText("F1048542"));
		assertEquals("266773",CalcUtil.getCellText("O1048562"));
		sleep(1);
		
		CalcUtil.selectRange("D1048541");
		typeKeys("10000<enter>");
		sleep(1);
		
		CalcUtil.selectRange("O1048562");
		calc.menuItem("Data->DataPilot->Refresh").select();
		assertEquals("266525",CalcUtil.getCellText("O1048562"));
		
	}

}
