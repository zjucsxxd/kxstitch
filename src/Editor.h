/********************************************************************************
 *	Copyright (C) 2010 by Stephen Allewell					*
 *	stephen@mirramar.adsl24.co.uk						*
 *										*
 *	This program is free software; you can redistribute it and/or modify	*
 *	it under the terms of the GNU General Public License as published by	*
 *	the Free Software Foundation; either version 2 of the License, or	*
 *	(at your option) any later version.					*
 ********************************************************************************/


#ifndef __Editor_H
#define __Editor_H


#include <QWidget>

#include "configuration.h"
#include "Editor.h"


class Document;
class Preview;
class Renderer;
class Scale;


class Editor : public QWidget
{
	Q_OBJECT

	public:
		enum SelectedStitchType {
			StitchQuarter,
			StitchHalf,
			Stitch3Quarter,
			StitchFull,
			StitchSmallHalf,
			StitchSmallFull,
			StitchFrenchKnot
		};

		enum ToolMode {
			ToolPaint,
			ToolDraw,
			ToolErase,
			ToolRectangle,
			ToolFillRectangle,
			ToolEllipse,
			ToolFillEllipse,
			ToolFillPolyline,
			ToolText,
			ToolSelect,
			ToolBackstitch,
			ToolPaste = 128		// or'd with the operating tool mode when pasting.
		};

		Editor(QWidget *);
		~Editor();

		void setDocument(Document *);
		Document *document();

		void setPreview(Preview *);

		void readDocumentSettings();

		Scale *horizontalScale();
		Scale *verticalScale();

		void resizeEditor();
		void zoom(double);

		QRect selectionArea();

		QList<int> visibleLayers();
		QList<int> layerOrder();

	signals:
		void selectionMade(bool);
		void changedVisibleCells(const QRect &);

	public slots:
		void renderStitches(bool);
		void renderBackstitches(bool);
		void renderFrenchKnots(bool);
		void renderGrid(bool);
		void renderBackgroundImages(bool);

		void renderStitchesAs();
		void renderBackstitchesAs();
		void renderKnotsAs();

		void colorHilight(bool);

		void selectTool();

		void selectStitch();

		void setMaskLayer(bool);
		void setMaskStitch(bool);
		void setMaskColor(bool);
		void setMaskBackstitch(bool);
		void setMaskKnot(bool);

	protected:
		void contextMenuEvent(QContextMenuEvent*);
		void dragEnterEvent(QDragEnterEvent*);
		void dragLeaveEvent(QDragLeaveEvent*);
		void dragMoveEvent(QDragMoveEvent*);
		void dropEvent(QDropEvent*);
		void keyPressEvent(QKeyEvent*);
		void keyReleaseEvent(QKeyEvent*);
		void mousePressEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void paintEvent(QPaintEvent*);

	private:
		void renderBackgroundImages(QPainter *, QRect);
		void renderGrid(QPainter *, QRect);
		void renderStitches(QPainter *, QRect);
		void renderBackstitches(QPainter *, QRect);
		void renderFrenchKnots(QPainter *, QRect);

		void renderRubberBandLine(QPainter *, QRect);
		void renderRubberBandRectangle(QPainter *, QRect);
		void renderRubberBandEllipse(QPainter *, QRect);

		void mousePressEvent_Paint(QMouseEvent*);
		void mouseMoveEvent_Paint(QMouseEvent*);
		void mouseReleaseEvent_Paint(QMouseEvent*);

		void mousePressEvent_Draw(QMouseEvent*);
		void mouseMoveEvent_Draw(QMouseEvent*);
		void mouseReleaseEvent_Draw(QMouseEvent*);

		void mousePressEvent_Erase(QMouseEvent*);
		void mouseMoveEvent_Erase(QMouseEvent*);
		void mouseReleaseEvent_Erase(QMouseEvent*);

		void mousePressEvent_Rectangle(QMouseEvent*);
		void mouseMoveEvent_Rectangle(QMouseEvent*);
		void mouseReleaseEvent_Rectangle(QMouseEvent*);

		void mousePressEvent_FillRectangle(QMouseEvent*);
		void mouseMoveEvent_FillRectangle(QMouseEvent*);
		void mouseReleaseEvent_FillRectangle(QMouseEvent*);

		void mousePressEvent_Ellipse(QMouseEvent*);
		void mouseMoveEvent_Ellipse(QMouseEvent*);
		void mouseReleaseEvent_Ellipse(QMouseEvent*);

		void mousePressEvent_FillEllipse(QMouseEvent*);
		void mouseMoveEvent_FillEllipse(QMouseEvent*);
		void mouseReleaseEvent_FillEllipse(QMouseEvent*);

		void mousePressEvent_FillPolyline(QMouseEvent*);
		void mouseMoveEvent_FillPolyline(QMouseEvent*);
		void mouseReleaseEvent_FillPolyline(QMouseEvent*);

		void mousePressEvent_Text(QMouseEvent*);
		void mouseMoveEvent_Text(QMouseEvent*);
		void mouseReleaseEvent_Text(QMouseEvent*);

		void mousePressEvent_Select(QMouseEvent*);
		void mouseMoveEvent_Select(QMouseEvent*);
		void mouseReleaseEvent_Select(QMouseEvent*);

		void mousePressEvent_Backstitch(QMouseEvent*);
		void mouseMoveEvent_Backstitch(QMouseEvent*);
		void mouseReleaseEvent_Backstitch(QMouseEvent*);

		QPoint contentsToCell(const QPoint &) const;
		int contentsToZone(const QPoint &) const;
		QPoint contentsToSnap(const QPoint &) const;
		QPoint snapToContents(const QPoint &) const;
		QRect cellToRect(QPoint);
		void processBitmap(QBitmap &, const QString &);
		QRect visibleCells();

		Document	*m_document;
		Preview		*m_preview;

		Renderer	*m_renderer;

		Scale		*m_horizontalScale;
		Scale		*m_verticalScale;

		int	m_cellWidth;
		int	m_cellHeight;
		int	m_cellHorizontalGrouping;
		int	m_cellVerticalGrouping;

		QList<int>	m_visibleLayers;
		QList<int>	m_layerOrder;

		enum ToolMode	m_toolMode;

		bool	m_renderBackgroundImages;
		bool	m_renderGrid;
		bool	m_renderStitches;
		bool	m_renderBackstitches;
		bool	m_renderFrenchKnots;

		bool	m_maskLayer;
		bool	m_maskStitch;
		bool	m_maskColor;
		bool	m_maskBackstitch;
		bool	m_maskKnot;

		QPoint	m_cellStart;
		QPoint	m_cellTracking;
		QPoint	m_cellEnd;

		int	m_zoneStart;
		int	m_zoneTracking;
		int	m_zoneEnd;

		enum SelectedStitchType	m_currentStitchType;

		Configuration::EnumRenderer_RenderStitchesAs::type	m_renderStitchesAs;
		Configuration::EnumRenderer_RenderBackstitchesAs::type	m_renderBackstitchesAs;
		Configuration::EnumRenderer_RenderKnotsAs::type		m_renderKnotsAs;
		bool							m_colorHilight;

		QRect	m_rubberBand;
		QRect	m_selectionArea;

		typedef void (Editor::*mouseEventCallPointer)(QMouseEvent*);
		typedef void (Editor::*renderToolSpecificGraphicsCallPointer)(QPainter *, QRect);

		static const mouseEventCallPointer	mousePressEventCallPointers[];
		static const mouseEventCallPointer	mouseMoveEventCallPointers[];
		static const mouseEventCallPointer	mouseReleaseEventCallPointers[];

		static const renderToolSpecificGraphicsCallPointer	renderToolSpecificGraphics[];
};


#endif