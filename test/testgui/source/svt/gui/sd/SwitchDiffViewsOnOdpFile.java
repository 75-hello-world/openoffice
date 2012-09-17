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

package svt.gui.sd;

import static org.openoffice.test.common.Testspace.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.gui.AppTool.*;
import static testlib.gui.UIMap.*;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.HashMap;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.Logger;
import org.openoffice.test.common.SystemUtil;
import org.openoffice.test.common.Testspace;

public class SwitchDiffViewsOnOdpFile {
	@Rule
	public Logger log = Logger.getLogger(this);
	
	private PrintStream result = null;
	
	private String pid = null;
	
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		OpenOffice.killAll();
		app.start();
		result = new PrintStream(new FileOutputStream(Testspace.getFile("output/svt_sd_switchViews.csv")));
		HashMap<String, Object> proccessInfo = SystemUtil.findProcess(".*(soffice\\.bin|soffice.*-env).*");
		pid = (String)proccessInfo.get("pid");
		result.println("Iterator,Time,Memory(KB),CPU(%)");
		log.info("Result will be saved to " + Testspace.getPath("output/svt_sd_switchViews.csv"));
	}

	@After
	public void tearDown() throws Exception {
		app.close();
		result.close();
	}
	
	@Test
	public void switchDiffViewsOnOdpFile() throws Exception {
		String file = prepareData("svt/complex.odp");
		for(int i = 0; i < 500; i++)
		{
			app.dispatch(".uno:Open");
			submitOpenDlg(file);
			impress.waitForExistence(10, 2);
			sleep(5);
			
			impress.menuItem("View->Outline").select();
			sleep(2);
			impressOutline.menuItem("View->Slide Sorter").select();
			sleep(10);
			impressSlideSorter.menuItem("View->Notes Page").select();
			sleep(2);
			impressHandoutView.menuItem("View->Master->Slide Master").select();
			sleep(2);
			impress.menuItem("View->Master->Notes Master").select();
			sleep(2);
			
			impressHandoutView.menuItem("View->Normal").select();
			sleep(2);
			
			impress.menuItem("File->Close").select();
			activeMsgBox.no();
			sleep(2);
			
			HashMap<String, Object> perfData = SystemUtil.getProcessPerfData(pid);
			String record = i + "," + System.currentTimeMillis() + "," + perfData.get("rss") + "," + perfData.get("pcpu");
			log.info("Record: " + record);
			result.println(record);
			result.flush();
			
			sleep(3);
			
		}
	}

}
