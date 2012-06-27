/************************************************************************
 *
 * Licensed Materials - Property of IBM.
 * (C) Copyright IBM Corporation 2003, 2012.  All Rights Reserved.
 * U.S. Government Users Restricted Rights:
 * Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 ************************************************************************/

package org.openoffice.test.vcl.widgets;

import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.SmartId;


public class VclListBox extends VclControl  {

	/**
	 * Construct the list box with its string ID
	 * @param uid
	 */
	public VclListBox(String uid) {
		super(uid);
	}

	public VclListBox(SmartId id) {
		super(id);
	}

	/**
	 * Returns the number of entries in a TreeListBox.
	 * 
	 * @return Number of list box entries. Error if the return value is -1.
	 */
	public int getItemCount() {
		return ((Long) invoke(Constant.M_GetItemCount)).intValue();
	}

	/**
	 * Get the text of the specified entry in the tree list box Notice:
	 * index,col starting from 0
	 * 
	 * @param index
	 * @param col
	 * @return
	 */
	public String getItemText(int index, int col) {
		return (String) invoke(Constant.M_GetItemText, new Object[] { new Integer(index + 1), new Integer(col + 1) });
	}

	/**
	 * Get the text of the specified node Notice: index starting from 0
	 * 
	 * @param index
	 * @return
	 */
	public String getItemText(int index) {
		return getItemText(index, 0);
	}

	/**
	 * Returns the number of selected entries in a TreeListbox(you can select
	 * more than one entry).
	 * 
	 * @return The number of selected entries. Error is the return value is -1.
	 */
	public int getSelCount() {
		return ((Long) invoke(Constant.M_GetSelCount)).intValue();
	}

	/**
	 * Returns the index number of the selected entry in the TreeListBox.
	 * Notice: index starting from 0
	 * 
	 * @return The index number of selected entries. Error is the return value
	 *         is -1.
	 */
	public int getSelIndex() {
		return ((Long) invoke(Constant.M_GetSelIndex)).intValue() - 1;
	}

	/**
	 * Get the text of the selected item
	 */
	public String getSelText() {
		return (String) invoke(Constant.M_GetSelText);
	}

	/**
	 * Select the specified node via its index Notice: index starting from 0
	 * 
	 * @param index
	 */
	public void select(int index) {
		invoke(Constant.M_Select, new Object[] { new Integer(index + 1) });
	}

	/**
	 * Selects the text of an entry.
	 * 
	 * @param str
	 *            the item string
	 */
	public void select(String text) {
		if (getType() == 324) {
			int count = getItemCount();
			for (int i = 0; i < count; i++) {
				if (text.equals(getItemText(i))) {
					select(i);
					return;
				}
			}
			
			throw new RuntimeException(text + " is not found in the list box");
		} else {
			invoke(Constant.M_Select, new Object[] { text });
		}
	}

	/**
	 * Append one item to be selected after selected some items.
	 * 
	 * @param i
	 *            the index of the item
	 */
	public void multiSelect(int i) {
		invoke(Constant.M_MultiSelect, new Object[] { new Integer(i + 1) });
	}

	/**
	 * Append one item to be selected after selected some items.
	 * 
	 * @param text
	 *            the text of the item
	 */
	public void multiSelect(String text) {
		invoke(Constant.M_MultiSelect, new Object[] { text });
	}

	/**
	 * get all items'text.
	 * 
	 */
	public String[] getItemsText() {
		int count = getItemCount();
		String[] res = new String[count];
		for (int i = 0; i < count; i++) {
			res[i] = getItemText(i);
		}
		return res;
	}

	/**
	 * 
	 * @param text
	 * @return
	 */
	public int getItemIndex(String text) {
		int count = getItemCount();
		for (int i = 0; i < count; i++) {
			if (text.equals(getItemText(i)))
				return i;
		}
		
		throw new RuntimeException(text + " is not found in the list box");
	}
	
	
	/**
	 * Check if the list box has the specified item
	 * 
	 * @param str
	 * @return
	 */
	public boolean hasItem(String str) {
		int len = getItemCount();
		for (int i = 0; i < len; i++) {
			String text = getItemText(i);
			if (str.equals(text))
				return true;
		}
		return false;
	}
}
