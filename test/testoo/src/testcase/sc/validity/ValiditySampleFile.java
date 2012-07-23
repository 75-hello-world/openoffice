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



package testcase.sc.validity;

import static testlib.AppUtil.submitOpenDlg;
import static testlib.AppUtil.testFile;
import static testlib.AppUtil.typeKeys;
import static testlib.UIMap.*;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.typeKeys;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import testlib.CalcUtil;
import testlib.Log;

public class ValiditySampleFile {

	@Rule
	public Log LOG = new Log();

	@Before
	public void setUp() throws Exception {
		app.start();
	}

	@After
	public void tearDown() throws Exception {
		app.close();
	}

	/**
	 * Test open MS 2003 spreadsheet with ignore blank validity.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testFFCIgnoreBlank() throws Exception{
		// Open sample file
		String file = testFile("sc/FFC252FFCSC_XML_Datarange0235.xls");
		app.dispatch(".uno:Open", 3);
		submitOpenDlg(file);
		calc.waitForExistence(10, 2);

		CalcUtil.selectRange("D5");
		SC_InputBar_Input.activate();
		for(int i=1;i<=10;i++)
			typeKeys("<backspace>");
		typeKeys("<enter>");

		assertEquals("",CalcUtil.getCellText("D5"));
	}

	/**
	 * Test open MS 2003 spreadsheet with ignore blank validity.
	 * 
	 * @throws Exception
	 */
	@Test
	public void testFFCNotIgnoreBlank() throws Exception{
		//open sample file
		String file = testFile("sc/FFC252FFCSC_XML_Datarange0205.xls");
		app.dispatch(".uno:Open", 3);
		submitOpenDlg(file);
		calc.waitForExistence(10, 2);

		CalcUtil.selectRange("F5");
		SC_InputBar_Input.activate();
		typeKeys("<backspace><enter>");

		assertEquals("Invalid value.",ActiveMsgBox.getMessage());
		ActiveMsgBox.ok();
		assertEquals("8",CalcUtil.getCellText("F5"));
	}

	/**
	 * test Cell is not locked after switch from validity cell to source cells
	 */
	@Test
	public void testNotLockCellFromValidityCell() {
		//open sample file on data path
		String file = testFile("sc/sampledata.ods");
		app.dispatch(".uno:Open", 3);
		submitOpenDlg(file);
		calc.waitForExistence(10, 2);

		CalcUtil.selectRange("Sheet1.F19");
		typeKeys("d<enter>");
		CalcUtil.selectRange("Sheet1.F17");
		typeKeys("Test<enter>");

		assertEquals("Test",CalcUtil.getCellText("F17"));
	}
}
