package org.apache.openoffice.ooxml.parser;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

/** A simple reader for the parse table data that allows simple filtering on the
 *  first word in each line.
 *  
 *  Lines that only contain comments or whitespace are ignored.
 *
 */
public class ParseTableReader
{
    public ParseTableReader (final File aFile)
    {
        maSections = new HashMap<>();
        
        try
        {
            final BufferedReader aReader = new BufferedReader(new FileReader(aFile));
            
            while (true)
            {
                final String sLine = aReader.readLine();
                if (sLine == null)
                    break;
                if (sLine.startsWith("#"))
                    continue;
                final String aParts[] = sLine.split("\\s+");
                
                GetSection(aParts[0]).add(aParts);
            }
            
            aReader.close();
        } 
        catch (final Exception aException)
        {
            throw new RuntimeException(aException);
        }
    }


    
    
    public Vector<String[]> GetSection (final String sSectionName)
    {
        Vector<String[]> aSection = maSections.get(sSectionName);
        if (aSection == null)
        {
            aSection = new Vector<>();
            maSections.put(sSectionName, aSection);
        }
        return aSection;
    }

    
    

    private final Map<String,Vector<String[]>> maSections;
}
