package xmipp.ij.commons;

import ij.IJ;
import ij.ImagePlus;
import java.awt.Image;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Arrays;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import xmipp.ij.commons.XmippUtil;

import xmipp.jni.ImageGeneric;


public class XmippUtil {

	private static XmippImageJ xij;

	public static XmippImageJ showImageJ(Tool tool) {
		if (IJ.getInstance() == null) {

            try {

                xij = new XmippImageJ();
//			
                File tempFile = File.createTempFile("macros", ".txt");
                BufferedWriter writer = new BufferedWriter(new FileWriter(tempFile));
                writer.write("macro \"Particle Picker Tool - C0a0L18f8L818f\" {   }\nmacro \"Xmipp Micrograph Viewer Tool - C0a0L18f8L818f\" {  }");
                writer.close();
                IJ.run("Install...", "install=" + tempFile.getAbsolutePath());
             } catch (Exception ex) {
                Logger.getLogger(XmippUtil.class.getName()).log(Level.SEVERE, null, ex);
                throw new IllegalArgumentException(ex);
            }
		} else if (!xij.isVisible())
			xij.setVisible(true);
		return xij;
	}

	public static XmippImageJ getXmippImageJ() {
		return xij;
	}
        
        

	public static ImagePlus getImagePlus(String file) {
		try {

			ImageGeneric ig = new ImageGeneric(file);
			ig.read(ImageGeneric.FIRST_IMAGE);
			ImagePlus imp = XmippImageConverter.convertToImagePlus(ig);
			ig.destroy();

			return imp;
		} catch (Exception e) {
			e.printStackTrace();
			throw new IllegalArgumentException(e.getMessage());
		}
	}
	
	public static Icon getImageIcon(ImagePlus imp, int width, int height)
	{

		Image image = imp.getImage().getScaledInstance(width, height, Image.SCALE_SMOOTH);
		Icon icon = new ImageIcon(image);

		return icon;
	}
        


    public static String executeCommand(String[] command) throws Exception {

        System.out.println(Arrays.toString(command));
        StringBuffer output = new StringBuffer();

        Process p;

        p = Runtime.getRuntime().exec(command);
        p.waitFor();
        BufferedReader reader
                = new BufferedReader(new InputStreamReader(p.getInputStream()));

       
        String line = "";
        while ((line = reader.readLine()) != null) {
            output.append(line + "\n");
        }
        reader = new BufferedReader(new InputStreamReader(p.getErrorStream()));
        
        while ((line = reader.readLine()) != null) {
            output.append(line + "\n");
        }
        return output.toString();
    }


    public static void copyFile(String source, String dest) throws IOException
    {
        copyFile(new File(source), new File(dest));
    }
    
    public static void copyFile(File source, File dest)
            throws IOException {
        InputStream input = null;
        OutputStream output = null;
        try {
            input = new FileInputStream(source);
            output = new FileOutputStream(dest);
            byte[] buf = new byte[1024];
            int bytesRead;
            while ((bytesRead = input.read(buf)) > 0) {
                output.write(buf, 0, bytesRead);
            }
        } finally {
            input.close();
            output.close();
        }
}
       
}
