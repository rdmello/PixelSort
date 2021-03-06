
#include <iostream>
#include <cmath>
#include <string>

// 
// Static Release flags 
// 
// #define QT_STATICPLUGIN
// #include <QtCore/QtPlugin>
// Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
// 

#include <QImage>

#include "PixelSortApp.hpp"

PixelSortApp::PixelSortApp(QApplication* parent):
    appPtr(parent),
    scene(this),
    view(&scene, this),
    dockwidget("Tools", this),
    dockwidget_mid(&dockwidget),
    vbox(&dockwidget_mid),
    importbutton("Import", &dockwidget_mid),
    fileimport(this),
    exportbutton("Export", &dockwidget_mid),
    fileexport(this),
    formlayout(),
    sortContainer(&dockwidget_mid),
    sortBox(),
    rSort("", &sortContainer),
    gSort("", &sortContainer),
    bSort("", &sortContainer),
    moveContainer(&dockwidget_mid),
    moveBox(),
    rMove("", &moveContainer),
    gMove("", &moveContainer),
    bMove("", &moveContainer),
    minColBtn("Min Color", &dockwidget_mid),
    maxColBtn("Max Color", &dockwidget_mid),
    minColDialog(QColor(0, 0, 0), &dockwidget_mid),
    maxColDialog(QColor(255, 255, 255), &dockwidget_mid),
    angleLabel("Angle"),
    angleDial(&dockwidget_mid),
    startX(&dockwidget_mid),
    startY(&dockwidget_mid),
    boxW(&dockwidget_mid),
    boxH(&dockwidget_mid),
    distX(&dockwidget_mid),
    distY(&dockwidget_mid),
    sortbutton("PixelSort", &dockwidget_mid),
    ZoomInBtn("Zoom In"),
    ZoomOutBtn("Zoom Out"),
    ZoomResetBtn("Reset View"),
    PanBtn("Pan"),
    fileMenu("File"),
    img(),
    drv(img)
{
    this->resize(800, 500);
    this->setWindowTitle("PixelSort app v2");
    this->setUnifiedTitleAndToolBarOnMac(true);

    this->setCentralWidget(&view);
        
    // set up dockwidget
    this->addDockWidget(Qt::LeftDockWidgetArea, &dockwidget);
    dockwidget.setAllowedAreas(Qt::LeftDockWidgetArea);
    dockwidget.setFeatures(QDockWidget::NoDockWidgetFeatures);
    dockwidget.setWidget(&dockwidget_mid);
     
    // Set up menubar actions
    QAction* openAction = fileMenu.addAction("&Open");
    QAction* saveAction = fileMenu.addAction("&Save");
    openAction->setShortcuts(QKeySequence::Open);
    saveAction->setShortcuts(QKeySequence::Save);
    
    // Set up menubar
    menuBar()->addMenu(&fileMenu);
    menuBar()->addAction(fileMenu.menuAction());
    menuBar()->show();

    // add import and export button to vbox
    vbox.addWidget(&importbutton);
    vbox.addWidget(&exportbutton);
   
    // file import dialog settings
    fileimport.setFileMode(QFileDialog::ExistingFile);
    fileimport.setAcceptMode(QFileDialog::AcceptOpen);
    fileimport.setNameFilter("Images (*.png *.tiff *.tif)");
    fileimport.fileSelected(imageFilePath);
    // reload image view after importing 
    QObject::connect(&fileimport, SIGNAL(fileSelected(QString)), this, SLOT(reloadImage(QString)));

    // file export dialog settings
    fileexport.setFileMode(QFileDialog::AnyFile);
    fileexport.setAcceptMode(QFileDialog::AcceptSave);
    // write image after selecting export filename
    QObject::connect(&fileexport, SIGNAL(fileSelected(QString)), this, SLOT(writeImage(QString)));

    // Set up openAction and saveAction
    QObject::connect(openAction, &QAction::triggered, &fileimport, &QFileDialog::exec);
    QObject::connect(saveAction, &QAction::triggered, &fileexport, &QFileDialog::exec);
   
    // Connect import and export button events
    QObject::connect(&importbutton, &QPushButton::clicked, openAction, &QAction::trigger);
    QObject::connect(&exportbutton, &QPushButton::clicked, saveAction, &QAction::trigger);

    // Set up the form layout widget 
    vbox.addLayout(&formlayout);
    formlayout.setLabelAlignment(Qt::AlignCenter);
    
    // add sort options buttons to formlayout
    sortContainer.setLayout(&sortBox);
    sortBox.setContentsMargins(0, 0, 0, 0);
    sortBox.addWidget(&rSort);
    sortBox.addWidget(&gSort);
    sortBox.addWidget(&bSort);
    rSort.setChecked(true);
    gSort.setChecked(true);
    bSort.setChecked(true);
    rSort.setStyleSheet("background-color: red;");
    gSort.setStyleSheet("background-color: green;");
    bSort.setStyleSheet("background-color: blue;");
    
    moveContainer.setLayout(&moveBox);
    moveBox.setContentsMargins(0, 0, 0, 0);
    moveBox.addWidget(&rMove);
    moveBox.addWidget(&gMove);
    moveBox.addWidget(&bMove);
    rMove.setChecked(true);
    gMove.setChecked(true);
    bMove.setChecked(true);
    rMove.setStyleSheet("background-color: red;");
    gMove.setStyleSheet("background-color: green;");
    bMove.setStyleSheet("background-color: blue;");
    
    formlayout.addRow("Sort", &sortContainer);
    formlayout.addRow("Move", &moveContainer);

    /* Matcher threshold dialogs */
    // we can't use the native macOS dialog here
    // since Qt won't use the preset color value unless turned off
    minColDialog.setOption(QColorDialog::DontUseNativeDialog);
    maxColDialog.setOption(QColorDialog::DontUseNativeDialog);
    minColDialog.setCurrentColor(QColor(0, 0, 0));
    maxColDialog.setCurrentColor(QColor(255, 255, 255));
    QObject::connect(&minColBtn, &QPushButton::clicked, &minColDialog, &QColorDialog::exec);
    QObject::connect(&maxColBtn, &QPushButton::clicked, &maxColDialog, &QColorDialog::exec);
    formlayout.addRow(&minColBtn);
    formlayout.addRow(&maxColBtn);

    /* Set angle control property */
    angleDial.setWrapping(true);
    angleDial.setMinimum(-2);
    angleDial.setMaximum(6);
    angleDial.setValue(0);
    formlayout.addRow(&angleLabel);
    formlayout.addRow(&angleDial);

    /* Set start X and Y properties */
    startX.setMinimum(0);
    startX.setMaximum(100000);
    startX.setSingleStep(100);
    startX.setValue(0);
    startY.setMinimum(0);
    startY.setMaximum(100000);
    startY.setSingleStep(100);
    startY.setValue(0);
    formlayout.addRow("StartX", &startX);
    formlayout.addRow("StartY", &startY);

    /* Set box width and height properties */
    boxW.setMinimum(1);
    boxW.setMaximum(100000);
    boxW.setSingleStep(100);
    boxW.setValue(200);
    boxH.setMinimum(1);
    boxH.setMaximum(100000);
    boxH.setSingleStep(100);
    boxH.setValue(200);
    formlayout.addRow("Width", &boxW);
    formlayout.addRow("Height", &boxH);

    /* Set repeat width and height properties */
    distX.setMinimum(200);
    distX.setMaximum(100000);
    distX.setSingleStep(100);
    distX.setValue(200);
    distY.setMinimum(200);
    distY.setMaximum(100000);
    distY.setSingleStep(100);
    distY.setValue(200);
    formlayout.addRow("RepeatX", &distX);
    formlayout.addRow("RepeatY", &distY);
    QObject::connect(&boxW, SIGNAL(valueChanged(int)), this, SLOT(distXset(int)));
    QObject::connect(&boxH, SIGNAL(valueChanged(int)), this, SLOT(distYset(int)));

    // add sort button to formlayout
    formlayout.addRow(&sortbutton);
    QObject::connect(&sortbutton, SIGNAL(clicked()), this, SLOT(sortButtonAction()));
 
    // add quit button to vbox
    // vbox.addWidget(&quitbutton);
    // QObject::connect(&quitbutton, SIGNAL(clicked()), appPtr, SLOT(quit()));

    // Read initial image file
    imageFilePath = "./ChromaticSort.app/Contents/resources/lake.png";
    img.load(imageFilePath);
    QPixmap newImg = QPixmap::fromImage(img);
    mainImg = scene.addPixmap(newImg);
    updateScene(newImg);
    opts.setImage(&drv);
    // scene.setSceneRect(0, 0, img.width(), img.height());
    // view.setSceneRect(0, 0, img.width(), img.height());
    // view.setDragMode(QGraphicsView::DragMode::ScrollHandDrag);

    statusBar()->showMessage("New image successfully loaded: " + imageFilePath);

    // Set up statusbar (at bottom of window)
    opts.notifyMe = this;
    statusBar()->showMessage("Ready");
}

void PixelSortApp::reloadImage(QString fileStr) 
{
    statusBar()->showMessage("Reading image");
 
    // Read image using QImage
    img.load(fileStr);
    QPixmap newImg = QPixmap::fromImage(img);
    mainImg->setPixmap(newImg);
    updateScene(newImg);
 
    statusBar()->showMessage("New image successfully loaded: " + fileStr);
}

void PixelSortApp::updateScene(QPixmap& newImg) {
    scene.update();
    scene.setSceneRect(newImg.rect());
    view.update();
    view.fitInView(mainImg, Qt::KeepAspectRatio);
}

void PixelSortApp::writeImage(QString fileStr) 
{
    statusBar()->showMessage("Writing image");
    img.save(fileStr);
    statusBar()->showMessage("Successfully written image: " + fileStr);
}

void PixelSortApp::sortButtonAction() 
{
    /* Pick up current options */
    opts.sortColors[0] = rSort.isChecked();
    opts.sortColors[1] = gSort.isChecked();
    opts.sortColors[2] = bSort.isChecked();
    opts.moveColors[0] = rMove.isChecked();
    opts.moveColors[1] = gMove.isChecked();
    opts.moveColors[2] = bMove.isChecked();

    QColor minCol = minColDialog.currentColor();
    QColor maxCol = maxColDialog.currentColor();
    opts.colorMatcher[0] = double(minCol.red()) / 255;
    opts.colorMatcher[2] = double(minCol.green()) / 255;
    opts.colorMatcher[4] = double(minCol.blue()) / 255;
    opts.colorMatcher[1] = double(maxCol.red()) / 255;
    opts.colorMatcher[3] = double(maxCol.green()) / 255;
    opts.colorMatcher[5] = double(maxCol.blue()) / 255;
        
    opts.theta = 45.0*double(angleDial.value());

    opts.rect[0] = startX.value();
    opts.rect[1] = startY.value();
    opts.rect[2] = boxW.value();
    opts.rect[3] = boxH.value();

    opts.Xrepeat[1] = distX.value();
    opts.Yrepeat[1] = distY.value();

    /* Sort and Apply */
    statusBar()->showMessage("Sorting image...");
    opts.doSort();

    /* Updating Qt::QPixmap */
    statusBar()->showMessage("Writing into QPixmap");
    appPtr->processEvents();

    QPixmap newImg = QPixmap::fromImage(img);
    mainImg->setPixmap(newImg);
    updateScene(newImg);
    statusBar()->showMessage("Ready");
}

void PixelSortApp::notify(const char* str)
{
    statusBar()->showMessage(str);
    appPtr->processEvents();
    // QPixmap newImg = QPixmap::fromImage(img);
    // mainImg->setPixmap(newImg);
    // updateScene(newImg);
}

void PixelSortApp::distXset(int newval)
{
    if (distX.value() < newval) {
        distX.setValue(newval);
    }
    distX.setMinimum(newval);
}

void PixelSortApp::distYset(int newval)
{
    if (distY.value() < newval) {
        distY.setValue(newval);
    }
    distY.setMinimum(newval);
}
