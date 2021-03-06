/*
 * Copyright (C) 2010-2015 by Stephen Allewell
 * steve.allewell@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "ImportImageDlg.h"

#include <QApplication>
#include <QPainter>
#include <QProgressDialog>

#include <KHelpClient>
#include <KLocalizedString>

#include "configuration.h"
#include "FlossScheme.h"
#include "SchemeManager.h"
#include "SymbolManager.h"
#include "SymbolLibrary.h"


const uchar alphaData[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a,
    0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52,
    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04,
    0x08, 0x06, 0x00, 0x00, 0x00, 0xa9, 0xf1, 0x9e,
    0x7e, 0x00, 0x00, 0x00, 0x04, 0x73, 0x42, 0x49,
    0x54, 0x08, 0x08, 0x08, 0x08, 0x7c, 0x08, 0x64,
    0x88, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59,
    0x73, 0x00, 0x00, 0x0b, 0x13, 0x00, 0x00, 0x0b,
    0x13, 0x01, 0x00, 0x9a, 0x9c, 0x18, 0x00, 0x00,
    0x00, 0x31, 0x49, 0x44, 0x41, 0x54, 0x08, 0x99,
    0x4d, 0xc1, 0xc1, 0x0d, 0xc0, 0x20, 0x0c, 0x04,
    0x30, 0x53, 0xb1, 0x51, 0xa6, 0x65, 0x2a, 0xc4,
    0x3e, 0x28, 0xd7, 0x4f, 0x1f, 0xb5, 0xc7, 0x5a,
    0x2b, 0x70, 0xce, 0xd1, 0xdd, 0x1e, 0x9f, 0x7b,
    0xaf, 0x24, 0xe6, 0xde, 0x1b, 0x54, 0x15, 0x98,
    0x49, 0xfc, 0xbd, 0x57, 0x37, 0x14, 0x37, 0x6c,
    0x77, 0x53, 0x2d, 0x00, 0x00, 0x00, 0x00, 0x49,
    0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};


ImportImageDlg::ImportImageDlg(QWidget *parent, const Magick::Image &originalImage)
    :   QDialog(parent),
        m_alphaSelect(nullptr),
        m_originalImage(originalImage)
{
    ui.setupUi(this);

    ui.FlossScheme->addItems(SchemeManager::schemes());

    m_originalSize = QSize(m_originalImage.columns(), m_originalImage.rows());
    QString caption = i18n("Import Image - Image Size %1 x %2 pixels", m_originalSize.width(), m_originalSize.height());
    setWindowTitle(caption);

    resetImportParameters();

    m_useFractionals = ui.UseFractionals->isChecked();
    m_ignoreColor = ui.IgnoreColor->isChecked();

    createImageMap();
    renderPixmap();
}


Magick::Image ImportImageDlg::convertedImage() const
{
    return m_convertedImage;
}


bool ImportImageDlg::ignoreColor() const
{
    return m_ignoreColor;
}


Magick::Color ImportImageDlg::ignoreColorValue() const
{
    return m_ignoreColorValue;
}


QString ImportImageDlg::flossScheme() const
{
    return ui.FlossScheme->currentText();
}


double ImportImageDlg::horizontalClothCount() const
{
    return ui.HorizontalClothCount->value();
}


double ImportImageDlg::verticalClothCount() const
{
    return ui.VerticalClothCount->value();
}


bool ImportImageDlg::useFractionals() const
{
    return m_useFractionals;
}


void ImportImageDlg::hideEvent(QHideEvent *event)
{
    KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).writeEntry(QStringLiteral("ImportImageDlg"), size());

    QDialog::hideEvent(event);
}


void ImportImageDlg::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).hasKey(QStringLiteral("ImportImageDlg"))) {
        resize(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).readEntry(QStringLiteral("ImportImageDlg"), QSize()));
    }
}


void ImportImageDlg::on_FlossScheme_currentIndexChanged(const QString&)
{
    createImageMap();
    renderPixmap();
}


void ImportImageDlg::on_UseMaximumColors_toggled(bool checked)
{
    m_useMaximumColors = checked;
}


void ImportImageDlg::on_MaximumColors_valueChanged(int)
{
    killTimer(m_timer);
    m_timer = startTimer(500);
}


void ImportImageDlg::on_IgnoreColor_toggled(bool checked)
{
    delete m_alphaSelect;
    m_alphaSelect = nullptr;

    m_ignoreColor = checked;
    renderPixmap();
}


void ImportImageDlg::on_ColorButton_clicked(bool)
{
    m_alphaSelect = new AlphaSelect(ui.ImagePreview);
    connect(m_alphaSelect, SIGNAL(clicked(QPoint)), this, SLOT(selectColor(QPoint)));
    m_alphaSelect->show();
}


void ImportImageDlg::on_HorizontalClothCount_valueChanged(double horizontalClothCount)
{
    clothCountChanged(horizontalClothCount, ui.VerticalClothCount->value());
}


void ImportImageDlg::on_VerticalClothCount_valueChanged(double verticalClothCount)
{
    clothCountChanged(ui.HorizontalClothCount->value(), verticalClothCount);
}


void ImportImageDlg::clothCountChanged(double horizontalClothCount, double verticalClothCount)
{
    double preferredSizeWidth = m_preferredSize.width();
    double preferredSizeHeight = m_preferredSize.height();
    QString suffix(i18n("Stitches"));
    Configuration::EnumDocument_UnitsFormat::type units = Configuration::document_UnitsFormat();

    if (units != Configuration::EnumDocument_UnitsFormat::Stitches) {
        preferredSizeWidth /= horizontalClothCount;
        preferredSizeHeight /= verticalClothCount;
        suffix = ui.HorizontalClothCount->suffix().right(2);
    }

    if (ui.ClothCountLink->isChecked()) {
        ui.VerticalClothCount->setValue(horizontalClothCount);
    }

    QString formattedSizeWidth = QString::fromLatin1("%1").arg(preferredSizeWidth, 0, 'g', 3);
    QString formattedSizeHeight = QString::fromLatin1("%1").arg(preferredSizeHeight, 0, 'g', 3);
    // TODO is there a better way of representing the dimensional values below for correct translations.
    ui.FinalSize->setText(QString(i18nc("%1 width, %2 height and %3 units", "%1 x %2 %3", QString::fromLatin1("%1").arg(preferredSizeWidth, 0, 'g', 3), QString::fromLatin1("%1").arg(preferredSizeHeight, 0, 'g', 3), suffix)));
}


void ImportImageDlg::on_ClothCountLink_clicked(bool checked)
{
    ui.ClothCountLink->setIcon((checked) ? QIcon::fromTheme(QStringLiteral("object-locked")) : QIcon::fromTheme(QStringLiteral("object-unlocked")));

    if (checked) {
        ui.VerticalClothCount->setValue(ui.HorizontalClothCount->value());
        ui.VerticalClothCount->setEnabled(false);
    } else {
        ui.VerticalClothCount->setEnabled(true);
    }
}


void ImportImageDlg::on_PatternScale_valueChanged(int)
{
    killTimer(m_timer);
    m_timer = startTimer(500);
}


void ImportImageDlg::on_UseFractionals_toggled(bool checked)
{
    m_useFractionals = checked;
    killTimer(m_timer);
    m_timer = startTimer(500);
}


void ImportImageDlg::calculateSizes()
{
    m_convertedImage = m_originalImage;
    m_preferredSize = m_originalSize * ui.PatternScale->value() / 100;
    QSize imageSize = m_preferredSize;

    if (m_useFractionals) {
        imageSize *= 2;
    }

    Magick::Geometry geometry(imageSize.width(), imageSize.height());
    geometry.percent(false);
    m_convertedImage.sample(geometry);
    on_HorizontalClothCount_valueChanged(ui.HorizontalClothCount->value());
}


void ImportImageDlg::createImageMap()
{
    FlossScheme *scheme = SchemeManager::scheme(ui.FlossScheme->currentText());
    m_colorMap = *(scheme->createImageMap());
}


void ImportImageDlg::renderPixmap()
{
    QPixmap alpha;
    alpha.loadFromData(alphaData, 143);

    ui.ImagePreview->setCursor(Qt::WaitCursor);
    calculateSizes();
    m_convertedImage.modifyImage();
    m_pixmap = QPixmap(m_convertedImage.columns(), m_convertedImage.rows());
    m_pixmap.fill();

    m_convertedImage.quantizeColorSpace(Magick::RGBColorspace);
    m_convertedImage.quantizeColors(ui.UseMaximumColors->isChecked() ?
                                    std::min(ui.MaximumColors->value(), SymbolManager::library(Configuration::palette_DefaultSymbolLibrary())->indexes().count()) :
                                    SymbolManager::library(Configuration::palette_DefaultSymbolLibrary())->indexes().count());
    m_convertedImage.quantize();
    m_convertedImage.map(m_colorMap);
    m_convertedImage.modifyImage();

    QPainter painter;
    painter.begin(&m_pixmap);
    painter.drawTiledPixmap(m_pixmap.rect(), alpha);
    int width = m_convertedImage.columns();
    int height = m_convertedImage.rows();
    int pixelCount = width * height;

    QProgressDialog progress(i18n("Rendering preview"), i18n("Cancel"), 0, pixelCount, this);
    progress.setWindowModality(Qt::WindowModal);

    Magick::Pixels cache(m_convertedImage);
    const Magick::PixelPacket *pixels = cache.getConst(0, 0, width, height);

    for (int dy = 0 ; dy < height ; dy++) {
        QApplication::processEvents();
        progress.setValue(dy * width);

        if (progress.wasCanceled()) {
            break;
        }

        for (int dx = 0 ; dx < width ; dx++) {
            Magick::PixelPacket packet = *pixels++;

            if (!(packet.opacity)) {
                if (!(m_ignoreColor && packet == m_ignoreColorValue)) {
#if MAGICKCORE_QUANTUM_DEPTH == 8
                    QColor color(packet.red, packet.green, packet.blue);
#else
                    QColor color(packet.red / 256, packet.green / 256, packet.blue / 256);
#endif
                    painter.setPen(QPen(color));
                    painter.drawPoint(dx, dy);
                }
            }
        }
    }

    painter.end();
    ui.ImagePreview->setPixmap(m_pixmap);
    ui.ImagePreview->setCursor(Qt::ArrowCursor);
}


void ImportImageDlg::timerEvent(QTimerEvent*)
{
    killTimer(m_timer);;
    renderPixmap();
}


void ImportImageDlg::pickColor()
{
    if (m_ignoreColor) {
        m_alphaSelect = new AlphaSelect(ui.ImagePreview);
        connect(m_alphaSelect, SIGNAL(clicked(QPoint)), this, SLOT(selectColor(QPoint)));
        m_alphaSelect->show();
    } else {
        delete m_alphaSelect;
        m_alphaSelect = nullptr;
    }
}


void ImportImageDlg::selectColor(const QPoint &p)
{
    QSize trueSize(m_convertedImage.columns(), m_convertedImage.rows());
    QRect pixmapRect = m_alphaSelect->pixmapRect();

    delete m_alphaSelect;
    m_alphaSelect = nullptr;

    // convert p that is relative to the ScaledImageLabel to the pixmap size
    int x = (((p.x() - pixmapRect.left()) * trueSize.width()) / pixmapRect.width());
    int y = (((p.y() - pixmapRect.top()) * trueSize.height()) / pixmapRect.height());

    x = (x < 0) ? 0 : std::min(x, trueSize.width());
    y = (y < 0) ? 0 : std::min(y, trueSize.height());

    m_ignoreColorValue = m_convertedImage.pixelColor(x, y);
    QPixmap swatch(ui.ColorButton->size());
#if MAGICKCORE_QUANTUM_DEPTH == 8
    swatch.fill(QColor(m_ignoreColorValue.redQuantum(), m_ignoreColorValue.greenQuantum(), m_ignoreColorValue.blueQuantum()));
#else
    swatch.fill(QColor(m_ignoreColorValue.redQuantum() / 256, m_ignoreColorValue.greenQuantum() / 256, m_ignoreColorValue.blueQuantum() / 256));
#endif
    ui.ColorButton->setIcon(swatch);

    renderPixmap();
}


void ImportImageDlg::on_DialogButtonBox_accepted()
{
    accept();
}


void ImportImageDlg::on_DialogButtonBox_rejected()
{
    reject();
}


void ImportImageDlg::on_DialogButtonBox_helpRequested()
{
    KHelpClient::invokeHelp(QStringLiteral("ImportImageDialog"), QStringLiteral("kxstitch"));
}


void ImportImageDlg::on_DialogButtonBox_clicked(QAbstractButton *button)
{
    if (ui.DialogButtonBox->button(QDialogButtonBox::Reset) == button) {
        // Reset values
        m_convertedImage = m_originalImage;
        renderPixmap();
    }
}


void ImportImageDlg::resetImportParameters()
{
    double horizontalClothCount = Configuration::editor_HorizontalClothCount();
    double verticalClothCount = Configuration::editor_VerticalClothCount();
    Configuration::EnumEditor_ClothCountUnits::type clothCountUnits = Configuration::editor_ClothCountUnits();
    ui.HorizontalClothCount->setSuffix((clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) ? i18nc("Per centimeter measurements", "/cm") : i18nc("Per inch measurements", "/in"));
    ui.VerticalClothCount->setSuffix((clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) ? i18nc("Per centimeter measurements", "/cm") : i18nc("Per inch measurements", "/in"));
    ui.HorizontalClothCount->setSingleStep((clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) ? 0.01 : 1.0);
    ui.VerticalClothCount->setSingleStep((clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) ? 0.01 : 1.0);
    ui.HorizontalClothCount->setValue(horizontalClothCount);
    ui.VerticalClothCount->setValue(verticalClothCount);
    ui.VerticalClothCount->setEnabled(false);
    ui.ClothCountLink->setChecked(Configuration::editor_ClothCountLink());
    ui.ClothCountLink->setIcon(QIcon::fromTheme(QStringLiteral("object-locked")));

    m_preferredSize = QSize(Configuration::document_Width(), Configuration::document_Height());
    int scaledWidth = m_preferredSize.width() * 100 / m_originalSize.width();
    int scaledHeight = m_preferredSize.height() * 100 / m_originalSize.height();
    int scale = std::min(scaledWidth, scaledHeight);

    QString scheme = Configuration::palette_DefaultScheme();

    if (SchemeManager::scheme(scheme) == nullptr) {
        scheme = SchemeManager::schemes().at(scheme.toInt());
    }

    ui.FlossScheme->setCurrentItem(scheme);
    ui.PatternScale->setValue(scale);
    ui.UseMaximumColors->setChecked(Configuration::import_UseMaximumColors());
    ui.MaximumColors->setValue(Configuration::import_MaximumColors());
    ui.MaximumColors->setMaximum(SymbolManager::library(Configuration::palette_DefaultSymbolLibrary())->indexes().count());
    ui.MaximumColors->setToolTip(QString(i18n("Colors limited to %1 due to the number of symbols available", ui.MaximumColors->maximum())));
}
