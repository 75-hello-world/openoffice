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

import static org.junit.Assert.*;
import static org.openoffice.test.vcl.Tester.*;
import static testlib.AppUtil.*;
import static testlib.UIMap.*;

import java.awt.Rectangle;

import org.junit.AfterClass;
import org.junit.Assert;
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
public class BVTFileType {

	@Rule
	public Log LOG = new Log();
	
	
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		initApp();
	}

	/**
	 * Test New/Save a text document
	 * @throws Exception
	 */
	@Test
	public void testSaveNewODT() throws Exception {
		saveNewDocument("helloworld_saveas.odt");
	}
	
	@Test
	public void testSaveNewOTT() throws Exception {
		saveNewDocument("helloworld_saveas.ott");
	}
	
	@Test
	public void testSaveNewSXW() throws Exception {
		saveNewDocument("helloworld_saveas.sxw");
	}
	
	@Test
	public void testSaveNewSTW() throws Exception {
		saveNewDocument("helloworld_saveas.stw");
	}
	
	@Test
	public void testSaveNewDOC() throws Exception {
		saveNewDocument("helloworld_saveas.doc");
	}
	
	@Test
	public void testSaveNewTXT() throws Exception {
		saveNewDocument("helloworld_saveas.txt");
	}
	
	private void saveNewDocument(String file) {
		String saveTo = fullPath("temp/" + file);
		//Create a new text document
		startcenter.menuItem("File->New->Text Document").select();
		sleep(3);
		
		// Input some text by keyboard
		writer.focus();
	
		String text = "~!@#$%^&*()_+QWERTYUIOP{}|:LKJHGFDSAZXCVBNM<>? ";
		typeText(text);
		writer.menuItem("Edit->Select All").select();
		app.setClipboard(".wrong");
		sleep(1);
		typeKeys("<$copy>");
		sleep(1);
		
		// Verify the text via system clip board
		Assert.assertEquals("The typed text into writer", text, app.getClipboard());
		
		// Set the text style
		writer.openContextMenu();
//		menuItem("Text Properties...").select();
		menuItem("Character...").select();
		EffectsPage.select();
		EffectsPage_Color.select("Magenta");
		EffectsPage.ok();
		sleep(2);
		
		//Save the text document
		writer.menuItem("File->Save As...").select();
		FileUtil.deleteFile(saveTo);
		submitSaveDlg(saveTo);	
		if (AlienFormatDlg.exists(3))
			AlienFormatDlg.ok();
		
		
		// Close it by clicking main menu
		writer.menuItem("File->Close").select();
		openStartcenter();
		// Reopen the saved file
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(saveTo);
		writer.waitForExistence(10, 2);
		
		writer.menuItem("Edit->Select All").select();
		app.setClipboard(".wrong");
		typeKeys("<$copy>");
		sleep(1);
		// Verify if the text still exists in the file
		Assert.assertEquals("The typed text into writer is saved!", text, app.getClipboard());
	}
	
	@Test
	public void testSaveNewODS() throws Exception {
		saveNewSpreadsheet("helloworld_saveas.ods");
	}
	
	@Test
	public void testSaveNewOTS() throws Exception {
		saveNewSpreadsheet("helloworld_saveas.ots");
	}
	
	
	@Test
	public void testSaveNewSXC() throws Exception {
		saveNewSpreadsheet("helloworld_saveas.sxc");
	}
	
	
	@Test
	public void testSaveNewSTC() throws Exception {
		saveNewSpreadsheet("helloworld_saveas.stc");
	}
	
//	@Test
//	public void testSaveNewCSV() throws Exception {
//		saveNewSpreadsheet("helloworld_saveas.csv");
//	}
	
	
	@Test
	public void testSaveNewXLS() throws Exception {
		saveNewSpreadsheet("helloworld_saveas.xls");
	}
	
	private void saveNewSpreadsheet(String file) {
		String saveTo = fullPath("temp/" + file);
		String text = "Hello Openoffice";
		startcenter.menuItem("File->New->Spreadsheet").select();
		calc.waitForExistence(10, 2);
		CalcUtil.selectRange("A65536");
		typeKeys(text);
		calc.menuItem("File->Save As...").select();
		FileUtil.deleteFile(saveTo);
		submitSaveDlg(saveTo);	
		if (AlienFormatDlg.exists(3))
			AlienFormatDlg.ok();
		// Close it by clicking main menu
		calc.menuItem("File->Close").select();
		openStartcenter();
		// Reopen the saved file
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(saveTo);
		calc.waitForExistence(10, 2);
		Assert.assertEquals("The typed text is saved!", text, CalcUtil.getCellText("A65536"));
	}
	
	@Test
	public void testSaveNewODP() throws Exception {
		saveNewPresentation("helloworld_saveas.odp");
	}
	
	@Test
	public void testSaveNewOTP() throws Exception {
		saveNewPresentation("helloworld_saveas.otp");
	}
	
	@Test
	public void testSaveNewPPT() throws Exception {
		saveNewPresentation("helloworld_saveas.ppt");
	}
	
	@Test
	public void testSaveNewPOT() throws Exception {
		saveNewPresentation("helloworld_saveas.pot");
	}
	
	
	@Test
	public void testSaveNewSXI() throws Exception {
		saveNewPresentation("helloworld_saveas.sxi");
	}
	
	@Test
	public void testSaveNewSTI() throws Exception {
		saveNewPresentation("helloworld_saveas.sti");
	}
	
	
	private void saveNewPresentation(String file) {
		String saveTo = fullPath("temp/" + file);
		String text = "Hello Openoffice";
		startcenter.menuItem("File->New->Presentation").select();
		PresentationWizard.ok();
		impress.click(0.01, 0.01);
		typeKeys(text);
		sleep(2);
		impress.doubleClick(0.1, 0.5);

		impress.menuItem("File->Save As...").select();
		FileUtil.deleteFile(saveTo);
		submitSaveDlg(saveTo);
		if (AlienFormatDlg.exists(3))
			AlienFormatDlg.ok();
		// Close it by clicking main menu
		impress.menuItem("File->Close").select();
		openStartcenter();
		// Reopen the saved file
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(saveTo);
		impress.waitForExistence(10, 2);
		sleep(2);
		impress.click(3, 3);
		typeKeys("<tab><enter>");
		impress.menuItem("Edit->Select All").select();
//		app.setClipboard(".wrong");
		typeKeys("<$copy>");
		sleep(1);
		Assert.assertEquals("The typed text is saved!", text,
				app.getClipboard().trim());
	}
	
	// drawing
	
	/**
	 * Test save a new drawing as .odg
	 */
	@Ignore("There is bug in draw")
	public void testSaveNewODG() throws Exception {
		saveNewDrawing("draw_saveas.odg");
	}
	
	/**
	 * Test save a new drawing as .otg
	 */
	@Ignore("There is bug in draw")
	public void testSaveNewOTG() throws Exception {
		saveNewDrawing("draw_saveas.otg");
	}
	
	/**
	 * Test save a new drawing as .sxd
	 */
	@Ignore("There is bug in draw")
	public void testSaveNewSXD() throws Exception {
		saveNewDrawing("draw_saveas.sxd");
	}
	
	/**
	 * Test save a new drawing as .std
	 */
	@Ignore("There is bug in draw")
	public void testSaveNewSTD() throws Exception {
		saveNewDrawing("draw_saveas.std");
	}
	
	/**
	 * New/Save a draw document
	 * 1. New a draw document
	 * 2. Insert a picture
	 * 3. Save it as the input filename
	 * 4. Reopen the saved file
	 * 5. Check if the picture is still there
	 * @param filename: filename to be saved
	 * @throws Exception
	 */
	public void saveNewDrawing(String filename) {
		String saveTo = fullPath("temp/" + filename);
		String bmp_green = testFile("pure_green_64x64.bmp");
		
		// Create a new drawing document
		startcenter.menuItem("File->New->Drawing").select();
		sleep(3);
		
		// Insert a picture fully filled with green
		draw.menuItem("Insert->Picture->From File...").select();
		submitOpenDlg(bmp_green);
		sleep(3);
		// Focus on edit pane
		draw.click(5,5);
		sleep(1);
		
		// Verify if the picture is inserted successfully
		Rectangle rectangle = GraphicsUtil.findRectangle(draw.getScreenRectangle(), 0xFF00FF00);
		assertNotNull("Green rectangle: " + rectangle, rectangle);
		
		// Save the drawing
		draw.menuItem("File->Save As...").select();
		FileUtil.deleteFile(saveTo);
		submitSaveDlg(saveTo);
		// If the format is supported by OO1.0, ask whether to change to the latest format
		if (AlienFormatDlg.exists(3))
			AlienFormatDlg.ok();	// Keep the current format
		
		// Close it by clicking main menu
		draw.menuItem("File->Close").select();
		openStartcenter();
		
		// Reopen the saved file
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(saveTo);
		draw.waitForExistence(10, 2);
		
		// Verify if the picture still exists in the file
		Rectangle rectangle1 = GraphicsUtil.findRectangle(draw.getScreenRectangle(), 0xFF00FF00);
		assertNotNull("Green rectangle: " + rectangle1, rectangle1);		
	}
	
	// math
	/**
	 * Test save a new math as .odf
	 */
	@Test
	public void testSaveNewODF() throws Exception{
		saveNewMath("math_saveas.odf");
	}
	
	/**
	 * Test save a new math as .sxm
	 */
	@Test
	public void testSaveNewSXM() throws Exception{
		saveNewMath("math_saveas.sxm");
	}
	
	/**
	 * Test save a new math as .mml
	 */
	@Test
	public void testSaveNewMML() throws Exception{
		saveNewMath("math_saveas.mml");
	}
	
	/**
	 * New/Save a math
	 * 1. New a math
	 * 2. Insert a formula
	 * 3. Save it as the input filename
	 * 4. Reopen the saved file
	 * 5. Check if the formula is still there
	 * @param filename: filename to be saved
	 * @throws Exception
	 */
	public void saveNewMath(String filename) {
		String saveTo = fullPath("temp/" + filename);
		
		// Create a new math
		startcenter.menuItem("File->New->Formula").select();
		sleep(3);
		
		// Verify if the Elements window is active		
		assertTrue(math_ElementsWindow.exists(3));
		
		// Insert a formula
		String text = "5 times 3 = 15";
		typeText(text);
		math_EditWindow.menuItem("Edit->Select All").select();
		typeKeys("<$copy>");
		sleep(1);
		
		// Verify the text via system clip board
		assertEquals("The typed formula into math", text, app.getClipboard());
		
		// Save the formula
		math_EditWindow.menuItem("File->Save As...").select();
		FileUtil.deleteFile(saveTo);
		submitSaveDlg(saveTo);
		// If the format is supported by OO1.0, ask whether to change to the latest format
		if (AlienFormatDlg.exists(3))
			AlienFormatDlg.ok();	// Keep the current format
	
		// Close it by clicking main menu
		math_EditWindow.menuItem("File->Close").select();
		openStartcenter();
		
		// Reopen the saved file
		startcenter.menuItem("File->Open...").select();
		submitOpenDlg(saveTo);
		math_EditWindow.waitForExistence(10, 2);
		
		// Verify if the formula still exists in the file
		math_EditWindow.menuItem("Edit->Select All").select();
		typeKeys("<$copy>");
		sleep(1);
		assertEquals("The typed formula into math is saved", text, app.getClipboard());
		
		// Close the file to avoid the app closing the Elements window automatically
		math_EditWindow.menuItem("File->Close").select();
	}
	
	@AfterClass
	public static void afterClass() {
		app.kill();
	}
}
