/***************************************************************************
 *
 * @author: Jesus Cuenca (jcuenca@cnb.csic.es)
 *
 * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 *  All comments concerning this program package may be sent to the
 *  e-mail address 'xmipp@cnb.csic.es'
 ***************************************************************************/

/**
 *   - Why?
 * Offer syntactic sugar to java.util.Properties
 */
public class Properties extends java.util.Properties {
	public Double getNumber(String key) {
		String s = getProperty(key);
		if (s != null) {
			try {
				return Double.valueOf(s);
			} catch (NumberFormatException e) {
			}
		}
		return null;
	}

	public double getDouble(String key) {
		Double n = getNumber(key);
		return n != null ? n.doubleValue() : 0.0;
	}

	public boolean getBoolean(String key) {
		String s = getProperty(key);
		return s != null && s.equals("true") ? true : false;
	}
}
