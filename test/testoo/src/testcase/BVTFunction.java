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
package testcase;

import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;
import java.io.File;

import org.junit.After;
import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.GraphicsUtil;

import testlib.CalcUtil;
import testlib.Log;

/**
 * 
 */
public class BVTFunction {

	
	@Rule
	public Log LOG = new Log();
	
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		initApp();
	}
	
	
	@Test
	public void testExportAsPDF() throws Exception  {
		String file = testFile("export_pdf.odt");
		String exportTo = fullPath("temp/odt.pdf");
		
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		writer.waitForExistence(10, 2);
		writer.menuItem("File->Export as PDF...").select();
		PDFGeneralPage.ok();
		FileUtil.deleteFile(exportTo);
		submitSaveDlg(exportTo);
		assertTrue("PDF is exported?", new File(exportTo).exists());
		
		// Via toolbar
		writer.menuItem("File->New->Text Document").select();
		assertTrue(toolbox(".HelpId:standardbar").exists(5));
		button(".uno:ExportDirectToPDF").click();
		assertEquals("PDF - Portable Document Format (.pdf)", FileSave_FileType.getSelText());
		FileSave.cancel();
		
	}
	
	/**
	 * Test the File -- Print Dialog  show 
	 * 
	 */	
	@Test
	public void testPrintDialog()
	{
		//Create a new text document
		startcenter.menuItem("File->New->Text Document").select();
		sleep(3);	
		writer.menuItem("File->Print").select();
		assertTrue(File_PrintDlg.exists(5));
		File_PrintDlg.cancel();
	}	
	
	/**
	 * Test the File -- Java Dialog  show 
	 * 
	 */	
	@Test
	public void testJavaDialog()
	{
		
		//Create a new text document and launch a Wizards dialog which need JVM work correctly.
		startcenter.menuItem("File->New->Text Document").select();
		File tempfile=new File(getUserInstallationDir(),"user/template/myAgendaTemplate.ott");
		FileUtil.deleteFile(tempfile);
		sleep(3);	
		writer.menuItem("File->Wizards->Agenda").select();
		sleep(5);	
		assertTrue(Wizards_AgendaDialog.exists(10));
		Wizards_AgendaDialog_FinishButton.click();
		sleep(10);	
		writer.focus();
		sleep(1);
		writer.menuItem("Edit->Select All").select();
		typeKeys("<$copy>");
		//System.out.println("now txt:"+app.getClipboard());
		assertTrue(app.getClipboard().startsWith("<Name>"));
	}	
	
	/**
	 * Test the Tools / Macros / Organize Dialogs" show 
	 * 
	 */	
	@Test
	public void testMacroToolsOrgDialog()
	{
		startcenter.menuItem("Tools->Macros->Organize Dialogs").select();
		assertTrue(MacroDialogsPage.exists(5));
		MacroDialogsPage.cancel();
	}
	
	
	/**
	 * Test the About Dialog show 
	 * 
	 */	
	@Test
	public void testAboutDialog()
	{
		startcenter.menuItem("Help->About OpenOffice.org").select();
		assertTrue(AboutDialog.exists(5));
		AboutDialog.ok();
	}
	
	/**
	 * Test inserting a picture in text document
	 * @throws Exception
	 */
	 
	 
	@Test
	public void testInsertPictureInDocument() throws Exception {
		String bmp_green = testFile("pure_green_64x64.bmp");
		String bmp_red = testFile("pure_red_64x64.bmp");
		
		//Create a new text document
		startcenter.menuItem("File->New->Text Document").select();
		sleep(3);
		
		//Insert a picture fully filled with green
		writer.click(400, 400);
		writer.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_green);
		sleep(3);
		writer.click(0.5, 0.5);
		sleep(1);
		
		// Verify if the picture is inserted successfully
		Rectangle rectangle = GraphicsUtil.findRectangle(writer.getScreenRectangle(), 0xFF00FF00);
		
		assertNotNull("Green rectangle: " + rectangle, rectangle);
		
		//insert another picture 
		writer.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_red);
		sleep(3);
		writer.click(0.5, 0.5);
		sleep(1);
		// Verify if the picture is inserted successfully
		rectangle = GraphicsUtil.findRectangle(writer.getScreenRectangle(), 0xFFFF0000);
		assertNotNull("Red rectangle: " + rectangle, rectangle);
	}


	@Test
	public void testInsertPictureInSpreadsheet() throws Exception {
		String bmp_green = testFile("pure_green_64x64.bmp");
		String bmp_red = testFile("pure_red_64x64.bmp");
		
		//Create a new text document
		startcenter.menuItem("File->New->Spreadsheet").select();
		sleep(3);
		
		//Insert a picture fully filled with green
		calc.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_green);
		sleep(3);
		calc.click(0.5, 0.5);
		sleep(1);
		
		// Verify if the picture is inserted successfully
		Rectangle rectangle = GraphicsUtil.findRectangle(calc.getScreenRectangle(), 0xFF00FF00);
		
		assertNotNull("Green rectangle: " + rectangle, rectangle);
//		assertEquals(new Rectangle(0,0,64,64), rectangle);
		CalcUtil.selectRange("C1");
		//insert another picture 
		calc.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_red);
		sleep(3);
		calc.click(0.5, 0.5);
		sleep(1);
		// Verify if the picture is inserted successfully
		rectangle = GraphicsUtil.findRectangle(calc.getScreenRectangle(), 0xFFFF0000);
		assertNotNull("Red rectangle: " + rectangle, rectangle);
	}
	
	@Test
	public void testInsertPictureInPresentation() throws Exception {
		String bmp_green = testFile("pure_green_64x64.bmp");
		String bmp_red = testFile("pure_red_64x64.bmp");
		
		//Create a new text document
		startcenter.menuItem("File->New->Presentation").select();
		sleep(3);
		
		//Insert a picture fully filled with green
		impress.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_green);
		sleep(3);
		impress.click(5,5);
		sleep(1);
		
		// Verify if the picture is inserted successfully
		Rectangle rectangle = GraphicsUtil.findRectangle(impress.getScreenRectangle(), 0xFF00FF00);
		
		assertNotNull("Green rectangle: " + rectangle, rectangle);
//		assertEquals(new Rectangle(0,0,64,64), rectangle);
		
		//insert another picture 
		impress.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_red);
		sleep(3);
		impress.click(1, 1);
		sleep(1);
		// Verify if the picture is inserted successfully
		rectangle = GraphicsUtil.findRectangle(impress.getScreenRectangle(), 0xFFFF0000);
		assertNotNull("Red rectangle: " + rectangle, rectangle);
	}
	
	@Test
	public void testSlideShow() throws Exception {
		String file = testFile("slideshow.odp");
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		impress.waitForExistence(10, 2);
		impress.menuItem("Slide Show->Slide Show").select();
		sleep(3);
		Rectangle rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFFFF0000);
		assertNotNull("1st slide appears", rectangle);
		SlideShow.click(0.5, 0.5);
		sleep(2);
		rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF00FF00);
		assertNotNull("2nd slide appears", rectangle);
		typeKeys("<enter>");
		sleep(2);
		rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF0000FF);
		assertNotNull("3rd slide appears", rectangle);
		SlideShow.click(0.5, 0.5);
		sleep(2);
		rectangle = GraphicsUtil.findRectangle(SlideShow.getScreenRectangle(), 0xFF0000FF);
		assertNull("The end", rectangle);
		SlideShow.click(0.5, 0.5);
		sleep(3);
		assertFalse("Quit", SlideShow.exists());
	}
	
	@Test
	public void testFind() {
		String file = testFile("find.odt");
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		writer.waitForExistence(10, 2);
		writer.menuItem("Edit->Find & Replace...").select();
		FindDlg_For.setText("OpenOffice");
		FindDlg_Find.click();
		sleep(1);
		writer.focus();
		typeKeys("<$copy>");
		assertEquals("OpenOffice", app.getClipboard());
		FindDlg_FindAll.click();
		sleep(1);
		writer.focus();
		typeKeys("<$copy>");
		assertEquals("OpenOfficeOpenOfficeOpenOffice", app.getClipboard());
		FindDlg_ReplaceWith.setText("Awesome OpenOffice");
		FindDlg_ReplaceAll.click();
		sleep(1);
		msgbox("Search key replaced 3 times.").ok();
		FindDlg.close();
		sleep(1);
		writer.menuItem("Edit->Select All").select();
		typeKeys("<$copy>");
		assertEquals("Apache Awesome OpenOffice is comprised of six personal productivity applications: a word processor (and its web-authoring component), spreadsheet, presentation graphics, drawing, equation editor, and database. Awesome OpenOffice is released on Windows, Solaris, Linux and Macintosh operation systems, with more communities joining, including a mature FreeBSD port. Awesome OpenOffice is localized, supporting over 110 languages worldwide. ", app.getClipboard());
	}
	
	@Test
	public void testFillInSpreadsheet() {
		String[][] expected1 = new String[][] {
				{"1"},
				{"1"},
				{"1"},
				{"1"},
				{"1"},
				{"1"},
		};
		String[][] expected2 = new String[][] {
				{"2"},
				{"2"},
				{"2"},
				{"2"},
				{"2"},
				{"2"},
		};
		
		String[][] expected3 = new String[][] {
				{"Hi friends","Hi friends","Hi friends", "Hi friends"}
		};
		
		String[][] expected4 = new String[][] {
				{"99999.999","99999.999","99999.999", "99999.999"}
		};
		String[][] expected5 = new String[][] {

		{ "99999.999", "-10" }, 
		{ "100000.999", "-9" }, 
		{ "100001.999", "-8" },
				{ "100002.999", "-7" }, 
			{ "100003.999", "-6" }

		};
		//Create a new text document
		startcenter.menuItem("File->New->Spreadsheet").select();
		sleep(3);
		
		CalcUtil.selectRange("C5");
		typeKeys("1<enter>");
		CalcUtil.selectRange("C5:C10");
		calc.menuItem("Edit->Fill->Down").select();
		assertArrayEquals("Fill Down:", expected1, CalcUtil.getCellTexts("C5:C10"));
		
		CalcUtil.selectRange("D10");
		typeKeys("2<enter>");
		CalcUtil.selectRange("D5:D10");
		calc.menuItem("Edit->Fill->Up").select();
		assertArrayEquals("Fill Up:", expected2, CalcUtil.getCellTexts("D5:D10"));
		
		CalcUtil.selectRange("A1");
		typeKeys("Hi friends<enter>");
		CalcUtil.selectRange("A1:D1");
		calc.menuItem("Edit->Fill->Right").select();
		assertArrayEquals("Fill Right:", expected3, CalcUtil.getCellTexts("A1:D1"));
		
		CalcUtil.selectRange("D2");
		typeKeys("99999.999<enter>");
		CalcUtil.selectRange("A2:D2");
		calc.menuItem("Edit->Fill->Left").select();
		assertArrayEquals("Fill left:", expected4, CalcUtil.getCellTexts("A2:D2"));

		CalcUtil.selectRange("E1");
		typeKeys("99999.999<tab>-10<enter>");
		
		CalcUtil.selectRange("E1:F5");
		calc.menuItem("Edit->Fill->Series...").select();
		assertTrue(button("1493549573").isChecked());
		assertFalse(button("1493549571").isChecked());
		dialog("26229").ok();
		sleep(1);
		assertArrayEquals("Fill series..", expected5, CalcUtil.getCellTexts("E1:F5"));
	}
	
	@Test
	public void testSort() {
		String[][] expected1 = new String[][] { 
				{ "-9999999" },
				{ "-1.1" },
				{ "-1.1" }, 
				{ "0" }, 
				{ "0" },
				{ "0.1" }, 
				{ "10" }, 
				{ "12" },
				{ "9999999" }, 
				{ "9999999" },

		};
		String[][] expected2 = new String[][] { 
				{ "TRUE", "Oracle" },
				{ "TRUE", "OpenOffice" }, 
				{ "FALSE", "OpenOffice" },
				{ "TRUE", "IBM" },
				{ "FALSE", "IBM" }, 
				{ "TRUE", "Google" },
				{ "FALSE", "facebook " },
				{ "TRUE", "Apache" },
				{ "TRUE", "!yahoo" }, 
				{ "TRUE", "" },

		};

		String[][] expected3 = new String[][] { { "Sunday" }, { "Monday" },
				{ "Tuesday" }, { "Wednesday" }, { "Thursday" }, { "Friday" },
				{ "Saturday" },

		};
		
		String[][] expected4 = new String[][] { { "-$10.00" }, { "$0.00" },
				{ "$0.00" }, { "$1.00" }, { "$3.00" }, { "$9.00" },
				{ "$123.00" }, { "$200.00" }, { "$400.00" }, { "$10,000.00" },

		};
		String file = testFile("sort.ods");
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(file);
		calc.waitForExistence(10, 2);
		CalcUtil.selectRange("A1:A10");
		calc.menuItem("Data->Sort...").select();
		assertEquals("Column A", listbox("956468740").getSelText());
		tabpage("58873").ok();
		sleep(1);
		assertArrayEquals("Sorted Data", expected1, CalcUtil.getCellTexts("A1:A10"));
		CalcUtil.selectRange("B1:C10");
		calc.menuItem("Data->Sort...").select();
		listbox("956468740").select("Column C");
		button("956465674").check();
		assertFalse(listbox("956468742").isEnabled());
		assertFalse(button("956465673").isEnabled());
		assertFalse(button("956465676").isEnabled());
		listbox("956468741").select("Column B");
		assertTrue(listbox("956468742").isEnabled());
		assertTrue(button("956465673").isEnabled());
		assertTrue(button("956465676").isEnabled());
		button("956465675").check();
		listbox("956468741").select("- undefined -");
		assertFalse(listbox("956468742").isEnabled());
		assertFalse(button("956465673").isEnabled());
		assertFalse(button("956465676").isEnabled());
		listbox("956468741").select("Column B");
		tabpage("58873").ok();
		sleep(1);
		
		assertArrayEquals("Sorted Data", expected2, CalcUtil.getCellTexts("B1:C10"));
		CalcUtil.selectRange("D1:D7");
		calc.menuItem("Data->Sort...").select();
		tabpage("58874").select();
		button("956482569").uncheck();
		button("956482567").check();
		listbox("956485122").select("Sunday,Monday,Tuesday,Wednesday,Thursday,Friday,Saturday");
		tabpage("58874").ok();
		sleep(1);
		assertArrayEquals("Sorted Data", expected3, CalcUtil.getCellTexts("D1:D7"));
		
		CalcUtil.selectRange("E1:E10");
		calc.menuItem("Data->Sort...").select();
		tabpage("58873").ok();
		sleep(1);
		assertArrayEquals("Sorted Data", expected4, CalcUtil.getCellTexts("E1:E10"));
	}
	
	@AfterClass
	public static void afterClass() {
		app.kill();
	}
}
