/***************************************************************************
 *
 * Authors:     Carlos Oscar S. Sorzano (coss@cnb.uam.es)
 *              Carlos Manzanares       (cmanzana@cnb.uam.es)
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
 *  e-mail address 'xmipp@cnb.uam.es'
 ***************************************************************************/

#include <cstring>

#include "file_menu.h"
#include "widget_micrograph.h"
#include "main_widget_mark.h"

#include <data/micrograph.h>
#include <data/funcs.h>

#include <qfiledialog.h>
#include <qgrid.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlineedit.h>

/* Constructor ------------------------------------------------------------- */
QtFileMenu::QtFileMenu(QtWidgetMicrograph* _parent) :
        QtPopupMenuMark(_parent)
{
    __coordinates_are_saved = TRUE;

    insertItem("Load coords", this, SLOT(slotLoadCoords()));
    insertItem("Save coords", this, SLOT(slotSaveCoords()));
    insertItem("Save angles", this, SLOT(slotSaveAngles()));
    insertItem("Generate images", this, SLOT(slotGenerateImages()));

    insertSeparator();
    options =  new QPopupMenu();
    insertItem("Change mark type", options);
    circle = options->insertItem("Circle");
    square = options->insertItem("Square");
    options->setCheckable(TRUE);
    connect(options, SIGNAL(activated(int)), this, SLOT(doOption(int)));
    setMouseTracking(TRUE);
    options->setItemChecked(circle, true);
    insertItem("Change mark radius", this, SLOT(slotChangeCircleRadius()));
    insertSeparator();

    insertItem("Quit", this, SLOT(slotQuit()));
}


/* Change mark type -------------------------------------------------------- */
void QtFileMenu::doOption(int item)
{

    Micrograph *m = ((QtWidgetMicrograph*)parentWidget())->getMicrograph();
    if (m == NULL) return;

    if (options->isItemChecked(item)) return;     // They are all radio buttons

    if (item == circle)
    {
        options->setItemChecked(circle, true);
        options->setItemChecked(square, false);
	((QtWidgetMicrograph*)parentWidget())->changeMarkType(MARK_CIRCLE);
    }
    else if (item == square)
    {
        options->setItemChecked(circle, false);
        options->setItemChecked(square, true);
	((QtWidgetMicrograph*)parentWidget())->changeMarkType(MARK_SQUARE);
    }
}

/* Change circle radius ---------------------------------------------------- */
void QtFileMenu::slotChangeCircleRadius()
{
    Micrograph *m = ((QtWidgetMicrograph*)parentWidget())->getMicrograph();
    if (m == NULL) return;
    ((QtWidgetMicrograph*)parentWidget())->slotChangeCircleRadius();
}

/* Load coordinates -------------------------------------------------------- */
void QtFileMenu::slotLoadCoords()
{
    Micrograph *m = ((QtWidgetMicrograph*)parentWidget())->getMicrograph();
    if (m == NULL) return;

    try
    {
        QFileDialog coordFileDialog(this, 0, TRUE);
        coordFileDialog.setCaption("Coordinates filename");
        coordFileDialog.setFilter("*.pos");
        if (coordFileDialog.exec())
        {
            // Get the family name from the filename
            FileName fn;
            fn = (char*)coordFileDialog.selectedFile().ascii();
            fn = fn.without_extension();
            fn = fn.remove_extension("raw");
            const char *familyName = fn.get_extension().c_str();
            emit signalAddFamily(familyName);

            int activeFamily = ((QtWidgetMicrograph*)parentWidget())->activeFamily();
            m->read_coordinates(activeFamily,
                                (char*)coordFileDialog.selectedFile().ascii());
            ((QtWidgetMicrograph*)parentWidget())->repaint();
            slotCoordChange();
        }
    }
    catch (Xmipp_error XE)
    {
        cout << XE;
    }
}

/* Save coordinates -------------------------------------------------------- */
void QtFileMenu::slotSaveCoords()
{
    Micrograph *m = ((QtWidgetMicrograph*)parentWidget())->getMicrograph();
    if (m == NULL) return;

    switch (QMessageBox::information(this, "Mark",
                                     "Save active family coordinates\n"
                                     "Are you sure?",
                                     "&Yes", "&No",
                                     0,      // Enter == button 0
                                     1))
    { // Escape == button 1
    case 0: // Yes clicked, Alt-Y or Enter pressed.
        break;
    case 1: // No clicked or Alt-N pressed or ESC
        return;
        break;
    }

    int activeFamily = ((QtWidgetMicrograph*)parentWidget())->activeFamily();
    m->write_coordinates(activeFamily,  m->micrograph_name() + "." +
                         m->get_label(activeFamily) +
                         ".pos");
    __coordinates_are_saved = TRUE;
}

/* Generate images --------------------------------------------------------- */
void QtFileMenu::slotGenerateImages()
{
    Micrograph *m = ((QtWidgetMicrograph*)parentWidget())->getMicrograph();
    if (m == NULL) return;
    /*
       switch(QMessageBox::information( this, "Mark",
                                       "Generate active family images\n"
                                       "Are you sure?",
                                       "&Yes", "&No",
                                       0,      // Enter == button 0
                                       1 ) ) { // Escape == button 1
           case 0: // Yes clicked, Alt-Y or Enter pressed.
               break;
           case 1: // No clicked or Alt-N pressed or ESC
               return; break;
        }
    */
    int activeFamily = ((QtWidgetMicrograph*)parentWidget())->activeFamily();
    try
    {
        QDialog   setPropertiesDialog(this, 0, TRUE);
        setPropertiesDialog.setCaption("Generate images");
        QGrid     qgrid(2, &setPropertiesDialog);
        qgrid.setMinimumSize(250, 170);
        QLabel    windowSizeXLabel("X window size: ", &qgrid);
        QLineEdit windowSizeXLineEdit(&qgrid);
        windowSizeXLineEdit.setText("100");
        QLabel    windowSizeYLabel("Y window size: ", &qgrid);
        QLineEdit windowSizeYLineEdit(&qgrid);
        windowSizeYLineEdit.setText("100");
        QLabel    rootNameLabel("Root name: ", &qgrid);
        QLineEdit rootNameLineEdit(&qgrid);
        QLabel    startingIndex("Starting index: ", &qgrid);
        QLineEdit startingIndexLineEdit(&qgrid);
        startingIndexLineEdit.setText("1");
        QLabel    originalM("Original micrograph: ", &qgrid);
        QLineEdit originalMLineEdit(&qgrid);
        QRadioButton  computeTransmitance("Transmitance -> O.D.  ", &qgrid);
        QRadioButton  computeInverse("Invert", &qgrid);
        computeTransmitance.setChecked(FALSE);
        computeInverse.setChecked(FALSE);
        QPushButton okButton("Ok", &qgrid);
        QPushButton cancelButton("Cancel", &qgrid);

        connect(&okButton, SIGNAL(clicked(void)),
                &setPropertiesDialog, SLOT(accept(void)));
        connect(&cancelButton, SIGNAL(clicked(void)),
                &setPropertiesDialog, SLOT(reject(void)));

        if (setPropertiesDialog.exec())
        {
            m->set_window_size(windowSizeXLineEdit.text().toInt(),
                               windowSizeYLineEdit.text().toInt());
            m->set_transmitance_flag(computeTransmitance.isChecked());
            m->set_inverse_flag(computeInverse.isChecked());
            if (!rootNameLineEdit.text().isEmpty())
            {
                // Select right angle
#define MAIN_WIDGET \
    ((QtMainWidgetMark *)(parentWidget()->parentWidget()))
                double alpha = 0, psi = 0, gamma = 0;
                bool this_is_tilted;
                if (MAIN_WIDGET->there_is_tilted())
                {
                    if (((QtWidgetMicrograph *)(MAIN_WIDGET->tilted_widget())) ==
                        (QtWidgetMicrograph *) parentWidget())
                    {
                        psi   = MAIN_WIDGET->alpha_t();
                        gamma = MAIN_WIDGET->gamma_t();
                        this_is_tilted = true;
                    }
                    else
                    {
                        gamma = 0;
                        alpha = MAIN_WIDGET->alpha_u();
                        this_is_tilted = false;
                    }
                }
                m->produce_all_images(activeFamily,
                                      (char*)rootNameLineEdit.text().ascii(),
                                      startingIndexLineEdit.text().toInt(),
                                      (char*)originalMLineEdit.text().ascii(),
                                      alpha, gamma, psi/*rot,tilt,psi*/);
                MAIN_WIDGET->generated(this_is_tilted,
                                       m->get_label(activeFamily));
            }
        }
    }
    catch (Xmipp_error XE)
    {
        cout << XE;
    }
}

/* Save angles ------------------------------------------------------------- */
void QtFileMenu::slotSaveAngles()
{
    switch (QMessageBox::information(this, "Mark",
                                     "Write micrograph angles\n"
                                     "Are you sure?",
                                     "&Yes", "&No",
                                     0,      // Enter == button 0
                                     1))
    { // Escape == button 1
    case 0: // Yes clicked, Alt-Y or Enter pressed.
        break;
    case 1: // No clicked or Alt-N pressed or ESC
        return;
        break;
    }

    ((QtMainWidgetMark *)(parentWidget()->parentWidget()))->write_angles();
}

/* Quit -------------------------------------------------------------------- */
void QtFileMenu::slotQuit()
{
    QtMainWidgetMark *M = (QtMainWidgetMark *) parentWidget()->parentWidget();
    // If there is no tilted micrograph there is nothing to do
    if (M->tilted_widget() != NULL)
    {
        M->compute_gamma();
        M->compute_alphas();
        slotSaveAngles();
    }

    if (!__coordinates_are_saved)
    {
        switch (QMessageBox::information(this, "Mark",
                                         "The document contains unsaved work\n"
                                         "Do you want to save it before exiting?",
                                         "&Save", "&Don't Save", "&Cancel",
                                         0,      // Enter == button 0
                                         2))
        { // Escape == button 2
        case 0: // Save clicked, Alt-S or Enter pressed.
            slotSaveCoords();
            exit(1);
            break;
        case 1: // Don't Save clicked or Alt-D pressed
            exit(1);
            break;
        case 2: // Cancel clicked, Alt-C or Escape pressed
            // don't do anything
            break;
        }
    }
    else exit(1);
}
