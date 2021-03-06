/*
 * Copyright (C) 2010-2015 by Stephen Allewell
 * steve.allewell@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */


#include "FilePropertiesDlg.h"

#include <QRect>

#include <KHelpClient>
#include <KLocalizedString>

#include "Commands.h"
#include "Document.h"
#include "Floss.h"
#include "FlossScheme.h"
#include "SchemeManager.h"


FilePropertiesDlg::FilePropertiesDlg(QWidget *parent, Document *document)
    :   QDialog(parent),
        m_document(document)
{
    setWindowTitle(i18n("File Properties"));
    ui.setupUi(this);

    QRect extents = m_document->pattern()->stitches().extents();
    m_minWidth = extents.width();
    m_minHeight = extents.height();
    m_width = m_document->pattern()->stitches().width();
    m_height = m_document->pattern()->stitches().height();
    m_horizontalClothCount = m_document->property(QStringLiteral("horizontalClothCount")).toDouble();
    m_verticalClothCount = m_document->property(QStringLiteral("verticalClothCount")).toDouble();
    m_clothCountLink = m_document->property(QStringLiteral("clothCountLink")).toBool();
    m_clothCountUnits = static_cast<Configuration::EnumEditor_ClothCountUnits::type>(m_document->property(QStringLiteral("clothCountUnits")).toInt());
    m_unitsFormat = static_cast<Configuration::EnumDocument_UnitsFormat::type>(m_document->property(QStringLiteral("unitsFormat")).toInt());
    ui.UnitsFormat->setCurrentIndex(m_unitsFormat);
    updatePatternSizes();
    ui.PatternTitle->setText(m_document->property(QStringLiteral("title")).toString());
    ui.PatternAuthor->setText(m_document->property(QStringLiteral("author")).toString());
    ui.PatternCopyright->setText(m_document->property(QStringLiteral("copyright")).toString());
    ui.PatternFabric->setText(m_document->property(QStringLiteral("fabric")).toString());
    ui.FabricColor->setColor(m_document->property(QStringLiteral("fabricColor")).value<QColor>());
    ui.FlossScheme->addItems(SchemeManager::schemes());
    ui.FlossScheme->setCurrentItem(m_document->pattern()->palette().schemeName());
    ui.Instructions->setPlainText(m_document->property(QStringLiteral("instructions")).toString());
    ui.ClothCountLink->setChecked(m_clothCountLink);
    on_ClothCountLink_clicked(m_clothCountLink);
}


int FilePropertiesDlg::documentWidth() const
{
    return m_width;
}


int FilePropertiesDlg::documentHeight() const
{
    return m_height;
}


Configuration::EnumDocument_UnitsFormat::type FilePropertiesDlg::unitsFormat() const
{
    return m_unitsFormat;
}


double FilePropertiesDlg::horizontalClothCount() const
{
    return m_horizontalClothCount;
}


bool FilePropertiesDlg::clothCountLink() const
{
    return ui.ClothCountLink->isChecked();
}


double FilePropertiesDlg::verticalClothCount() const
{
    return m_verticalClothCount;
}


Configuration::EnumEditor_ClothCountUnits::type FilePropertiesDlg::clothCountUnits() const
{
    return m_clothCountUnits;
}


QString FilePropertiesDlg::title() const
{
    return ui.PatternTitle->text();
}


QString FilePropertiesDlg::author() const
{
    return ui.PatternAuthor->text();
}


QString FilePropertiesDlg::copyright() const
{
    return ui.PatternCopyright->text();
}


QString FilePropertiesDlg::fabric() const
{
    return ui.PatternFabric->text();
}


QColor FilePropertiesDlg::fabricColor() const
{
    return ui.FabricColor->color();
}


QString FilePropertiesDlg::instructions() const
{
    return ui.Instructions->toPlainText();
}


QString FilePropertiesDlg::flossScheme() const
{
    return ui.FlossScheme->currentText();
}


void FilePropertiesDlg::hideEvent(QHideEvent *event)
{
    KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).writeEntry(QStringLiteral("FilePropertiesDlg"), size());

    QDialog::hideEvent(event);
}


void FilePropertiesDlg::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).hasKey(QStringLiteral("FilePropertiesDlg"))) {
        resize(KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("DialogSizes")).readEntry(QStringLiteral("FilePropertiesDlg"), QSize()));
    }
}


void FilePropertiesDlg::on_UnitsFormat_activated(int index)
{
    m_unitsFormat = static_cast<Configuration::EnumDocument_UnitsFormat::type>(index);

    switch (m_unitsFormat) {
    case Configuration::EnumDocument_UnitsFormat::Stitches:
        break;

    case Configuration::EnumDocument_UnitsFormat::Inches:
        if (m_clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) {
            m_horizontalClothCount *= 2.54;
            m_verticalClothCount *= 2.54;
        }

        m_clothCountUnits = Configuration::EnumEditor_ClothCountUnits::Inches;
        break;

    case Configuration::EnumDocument_UnitsFormat::CM:
        if (m_clothCountUnits == Configuration::EnumEditor_ClothCountUnits::Inches) {
            m_horizontalClothCount /= 2.54;
            m_verticalClothCount /= 2.54;
        }

        m_clothCountUnits = Configuration::EnumEditor_ClothCountUnits::CM;
        break;

    default: // Avoid compilation warnings about unhandled values
        break;
    }

    updatePatternSizes();
}


void FilePropertiesDlg::on_PatternWidth_valueChanged(double d)
{
    if (m_unitsFormat != Configuration::EnumDocument_UnitsFormat::Stitches) {
        d *= m_horizontalClothCount;
    }

    m_width = std::max((int)d, m_minWidth);
}


void FilePropertiesDlg::on_PatternHeight_valueChanged(double d)
{
    if (m_unitsFormat != Configuration::EnumDocument_UnitsFormat::Stitches) {
        d *= m_horizontalClothCount;
    }

    m_height = std::max((int)d, m_minHeight);
}


void FilePropertiesDlg::on_HorizontalClothCount_valueChanged(double d)
{
    m_horizontalClothCount = d;

    if (ui.ClothCountLink->isChecked()) {
        ui.VerticalClothCount->setValue(d);
    }

    if (m_unitsFormat != Configuration::EnumDocument_UnitsFormat::Stitches) {
        ui.PatternWidth->setValue(m_width / m_horizontalClothCount);
    }
}


void FilePropertiesDlg::on_VerticalClothCount_valueChanged(double d)
{
    m_verticalClothCount = d;

    if (m_unitsFormat != Configuration::EnumDocument_UnitsFormat::Stitches) {
        ui.PatternHeight->setValue(m_height / m_verticalClothCount);
    }
}


void FilePropertiesDlg::on_ClothCountLink_clicked(bool checked)
{
    ui.ClothCountLink->setIcon((checked) ? QIcon::fromTheme(QStringLiteral("object-locked")) : QIcon::fromTheme(QStringLiteral("object-unlocked")));

    if (checked) {
        ui.VerticalClothCount->setValue(ui.HorizontalClothCount->value());
        ui.VerticalClothCount->setEnabled(false);
    } else {
        ui.VerticalClothCount->setEnabled(true);
    }
}


void FilePropertiesDlg::on_DialogButtonBox_accepted()
{
    accept();
}


void FilePropertiesDlg::on_DialogButtonBox_rejected()
{
    reject();
}


void FilePropertiesDlg::on_DialogButtonBox_helpRequested()
{
    KHelpClient::invokeHelp(QStringLiteral("PatternPropertiesDialog"), QStringLiteral("kxstitch"));
}


void FilePropertiesDlg::updatePatternSizes()
{
    ui.UnitsFormat->setCurrentIndex(m_unitsFormat);

    double horizontalScale = (m_unitsFormat == Configuration::EnumDocument_UnitsFormat::Stitches) ? 1 : m_horizontalClothCount;
    double verticalScale = (m_unitsFormat == Configuration::EnumDocument_UnitsFormat::Stitches) ? 1 : m_verticalClothCount;
    double scaledMinWidth = m_minWidth / horizontalScale;
    double scaledMinHeight = m_minHeight / verticalScale;
    double scaledWidth = m_width / horizontalScale;
    double scaledHeight = m_height / verticalScale;

    ui.PatternWidth->setMinimum(scaledMinWidth);
    ui.PatternHeight->setMinimum(scaledMinHeight);
    ui.PatternWidth->setValue(scaledWidth);
    ui.PatternHeight->setValue(scaledHeight);
    ui.HorizontalClothCount->setValue(m_horizontalClothCount);
    ui.VerticalClothCount->setValue(m_verticalClothCount);

    QString suffix = ((m_clothCountUnits == Configuration::EnumEditor_ClothCountUnits::CM) ? i18nc("Per centimeter measurements", "/cm") : i18nc("Per inch measurements", "/in"));
    ui.HorizontalClothCount->setSuffix(suffix);
    ui.VerticalClothCount->setSuffix(suffix);

    double step = ((m_unitsFormat == Configuration::EnumDocument_UnitsFormat::Stitches) ? 1 : 0.01);
    ui.PatternWidth->setSingleStep(step);
    ui.PatternHeight->setSingleStep(step);
}
