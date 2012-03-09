/********************************************************************************
 *	Copyright (C) 2010 by Stephen Allewell					*
 *	stephen@mirramar.adsl24.co.uk						*
 *										*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or	*
 *	(at your option) any later version.					*
 ********************************************************************************/


#ifndef PrinterConfiguration_H
#define PrinterConfiguration_H


#include <QDataStream>
#include <QExplicitlySharedDataPointer>
#include <QList>

#include "Page.h"


#include "PrinterConfiguration_p.h"


class PrinterConfiguration
{
	public:
		PrinterConfiguration();
		PrinterConfiguration(const PrinterConfiguration &);
		~PrinterConfiguration();

		void clear();
		void detach();

		void addPage(Page *);
		void insertPage(int, Page *);
		void removePage(Page *);

		QList<Page *> &pages();

		friend QDataStream &operator<<(QDataStream &, const PrinterConfiguration &);
		friend QDataStream &operator>>(QDataStream &, PrinterConfiguration &);

	private:
		void updatePageNumbers();

		static const int version = 100;

		QExplicitlySharedDataPointer<PrinterConfigurationPrivate>	d;
};


#endif // PrinterConfiguration_H