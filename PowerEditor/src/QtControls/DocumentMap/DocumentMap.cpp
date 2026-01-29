// This file is part of Notepad++ project
// Copyright (C)2024 Notepad++ contributors

// This program is free software: you are free to redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.

#include "DocumentMap.h"

// Platform types for RECT definition
#include "../../MISC/Common/LinuxTypes.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QColor>
#include <QtGui/QPen>
#include <QtCore/QRect>

// Scintilla includes
#include "../../ScintillaComponent/ScintillaEditView.h"
#include "../../ScintillaComponent/Buffer.h"
#include "../../Parameters.h"

// Scintilla message constants
#ifndef SCI_GETZOOM
#define SCI_GETZOOM 2374
#endif
#ifndef SCI_SETZOOM
#define SCI_SETZOOM 2373
#endif
#ifndef SCI_SETVSCROLLBAR
#define SCI_SETVSCROLLBAR 2280
#endif
#ifndef SCI_SETHSCROLLBAR
#define SCI_SETHSCROLLBAR 2281
#endif
#ifndef SCI_SETMODEVENTMASK
#define SCI_SETMODEVENTMASK 2359
#endif
#ifndef SCI_GETDOCPOINTER
#define SCI_GETDOCPOINTER 2355
#endif
#ifndef SCI_SETDOCPOINTER
#define SCI_SETDOCPOINTER 2356
#endif
#ifndef SCI_GETFIRSTVISIBLELINE
#define SCI_GETFIRSTVISIBLELINE 2152
#endif
#ifndef SCI_LINESONSCREEN
#define SCI_LINESONSCREEN 2370
#endif
#ifndef SCI_DOCLINEFROMVISIBLE
#define SCI_DOCLINEFROMVISIBLE 2221
#endif
#ifndef SCI_POINTYFROMPOSITION
#define SCI_POINTYFROMPOSITION 2164
#endif
#ifndef SCI_POSITIONFROMPOINT
#define SCI_POSITIONFROMPOINT 2022
#endif
#ifndef SCI_GOTOPOS
#define SCI_GOTOPOS 2025
#endif
#ifndef SCI_GOTOLINE
#define SCI_GOTOLINE 2024
#endif
#ifndef SCI_TEXTHEIGHT
#define SCI_TEXTHEIGHT 2279
#endif
#ifndef SCI_LINESCROLL
#define SCI_LINESCROLL 2168
#endif
#ifndef SCI_GETLENGTH
#define SCI_GETLENGTH 2006
#endif
#ifndef SCI_GETCURRENTPOS
#define SCI_GETCURRENTPOS 2008
#endif
#ifndef SCI_LINEFROMPOSITION
#define SCI_LINEFROMPOSITION 2166
#endif
#ifndef SCI_POSITIONFROMLINE
#define SCI_POSITIONFROMLINE 2167
#endif
#ifndef SCI_GETWRAPMODE
#define SCI_GETWRAPMODE 2269
#endif
#ifndef SCI_SETWRAPMODE
#define SCI_SETWRAPMODE 2268
#endif
#ifndef SCI_GETWRAPINDENTMODE
#define SCI_GETWRAPINDENTMODE 2473
#endif
#ifndef SCI_SETWRAPINDENTMODE
#define SCI_SETWRAPINDENTMODE 2472
#endif
#ifndef SCI_SETMARGINLEFT
#define SCI_SETMARGINLEFT 2155
#endif
#ifndef SCI_SETMARGINRIGHT
#define SCI_SETMARGINRIGHT 2157
#endif
#ifndef SCI_COLOURISE
#define SCI_COLOURISE 4003
#endif
#ifndef SC_WRAP_WORD
#define SC_WRAP_WORD 1
#endif
#ifndef MODEVENTMASK_OFF
#define MODEVENTMASK_OFF 0
#endif

// Helper functions to convert between QRect and RECT
namespace {
    inline RECT QRectToRECT(const QRect& qr) {
        RECT rc;
        rc.left = qr.left();
        rc.top = qr.top();
        rc.right = qr.right();
        rc.bottom = qr.bottom();
        return rc;
    }

    inline QRect RECTToQRect(const RECT& rc) {
        return QRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
    }
}

namespace QtControls {

// ============================================================================
// Zoom ratio table for wrapping calculations
// Maps zoom level (-10 to 20) to width ratio
// ============================================================================
static constexpr double zoomRatio[] = {
    1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.5, 2.5, 3.5, 3.5,  // -10 to -1
    4.0, 4.5, 5.0, 5.0, 5.5, 6.0, 6.5, 7.0, 7.0, 7.5,  // 0 to 9
    8.0, 8.5, 8.5, 9.5, 9.5, 10.0, 10.5, 11.0, 11.0, 11.5, 12.0  // 10 to 20
};

// ============================================================================
// ViewZoneWidget Implementation
// ============================================================================
ViewZoneWidget::ViewZoneWidget(QWidget* parent)
    : QWidget(parent)
    , _focusColor(QColor(0xFF, 0x80, 0x00))   // Orange - DEFAULT_FOCUS_COLOR
    , _frostColor(QColor(0xFF, 0xFF, 0xFF))   // White - DEFAULT_FROST_COLOR
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void ViewZoneWidget::setZone(int higherY, int lowerY)
{
    _higherY = higherY;
    _lowerY = lowerY;
    update();
}

void ViewZoneWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);

    // Fill background with semi-transparent frost color
    QColor frost = _frostColor;
    frost.setAlpha(128);
    painter.fillRect(rect(), frost);

    // Draw the focus zone (visible region)
    if (_lowerY > _higherY) {
        QColor focus = _focusColor;
        focus.setAlpha(180);
        QRect zoneRect(0, _higherY, width(), _lowerY - _higherY);
        painter.fillRect(zoneRect, focus);

        // Draw border around the zone
        QPen pen(_focusColor);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawRect(zoneRect.adjusted(1, 1, -1, -1));
    }
}

void ViewZoneWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _dragging = true;
        emit zoneClicked(event->pos().y());
    }
}

void ViewZoneWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (_dragging && (event->buttons() & Qt::LeftButton)) {
        emit zoneDragged(event->pos().y());
    }
}

void ViewZoneWidget::mouseReleaseEvent(QMouseEvent* /*event*/)
{
    _dragging = false;
}

// ============================================================================
// DocumentMap Implementation
// ============================================================================
DocumentMap::DocumentMap(QWidget* parent)
    : StaticDialog(parent)
{
}

DocumentMap::~DocumentMap()
{
    // Clean up will be handled by parent widget hierarchy
}

void DocumentMap::init(ScintillaEditView** ppEditView)
{
    _ppEditView = ppEditView;
}

void DocumentMap::setupUI()
{
    QDialog* dialog = getDialog();
    if (!dialog) return;

    dialog->setWindowTitle(tr("Document Map"));
    dialog->resize(200, 400);

    _mainLayout = new QVBoxLayout(dialog);
    _mainLayout->setSpacing(4);
    _mainLayout->setContentsMargins(4, 4, 4, 4);

    // Map container frame
    _mapContainer = new QFrame(dialog);
    _mapContainer->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    auto* containerLayout = new QVBoxLayout(_mapContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // Note: The actual Scintilla widget (_mapView) would be created by the
    // main application and embedded here. For now we create a placeholder.
    // The ScintillaEditView would be set up via init() and the main app
    // would embed it into _mapContainer.

    // View zone overlay for highlighting visible region
    _viewZone = new ViewZoneWidget(_mapContainer);
    _viewZone->setFocusColor(DEFAULT_FOCUS_COLOR);
    _viewZone->setFrostColor(DEFAULT_FROST_COLOR);
    containerLayout->addWidget(_viewZone);

    _mainLayout->addWidget(_mapContainer, 1);  // Stretch factor 1

    // Zoom controls
    auto* zoomLayout = new QHBoxLayout();
    zoomLayout->setSpacing(8);

    auto* zoomTitleLabel = new QLabel(tr("Zoom:"), dialog);
    zoomLayout->addWidget(zoomTitleLabel);

    _zoomSlider = new QSlider(Qt::Horizontal, dialog);
    _zoomSlider->setRange(-10, 20);
    _zoomSlider->setValue(_zoomLevel);
    _zoomSlider->setTickPosition(QSlider::TicksBelow);
    _zoomSlider->setTickInterval(5);
    zoomLayout->addWidget(_zoomSlider, 1);

    _zoomLabel = new QLabel(QString::number(_zoomLevel), dialog);
    _zoomLabel->setMinimumWidth(30);
    zoomLayout->addWidget(_zoomLabel);

    _mainLayout->addLayout(zoomLayout);

    // Store initial rect
    _rc = dialog->geometry();
}

void DocumentMap::connectSignals()
{
    if (_zoomSlider) {
        connect(_zoomSlider, &QSlider::valueChanged,
                this, &DocumentMap::onZoomChanged);
    }

    if (_viewZone) {
        connect(_viewZone, &ViewZoneWidget::zoneClicked,
                this, &DocumentMap::onMapClicked);
        connect(_viewZone, &ViewZoneWidget::zoneDragged,
                this, &DocumentMap::onMapScrolled);
    }
}

void DocumentMap::doDialog()
{
    if (!isCreated()) {
        create(tr("Document Map"), false);
        setupUI();
        connectSignals();

        // Initialize the map view if we have a parent that can create it
        // This would typically be done by the main window
        if (_ppEditView && *_ppEditView) {
            // Set up the map view with default settings
            // The actual Scintilla widget creation would be handled by the main app
            reloadMap();
        }
    }

    display(true);
}

bool DocumentMap::run_dlgProc(QEvent* event)
{
    (void)event;
    return false;
}

void DocumentMap::reloadMap()
{
    if (_mapView && _ppEditView && *_ppEditView) {
        // Get the document pointer from the main editor
        auto currentDoc = (*_ppEditView)->execute(SCI_GETDOCPOINTER);
        _mapView->execute(SCI_SETDOCPOINTER, 0, currentDoc);

        // Sync with the current document buffer
        Buffer* editBuf = (*_ppEditView)->getCurrentBuffer();
        _mapView->setCurrentBuffer(editBuf);

        // Sync folding state
        std::vector<size_t> lineStateVector;
        (*_ppEditView)->getCurrentFoldStates(lineStateVector);
        _mapView->syncFoldStateWith(lineStateVector);

        // Handle wrapping
        if ((*_ppEditView)->isWrap() && needToRecomputeWith()) {
            wrapMap();
        }

        scrollMap();
        setSyntaxHighlighting();
    }
}

void DocumentMap::showInMapTemporarily(Buffer* buf2show, ScintillaEditView* fromEditView)
{
#ifndef NPP_LINUX
    // Windows implementation uses Buffer methods not available in QtCore::Buffer
    if (_mapView && fromEditView && buf2show) {
        _mapView->execute(SCI_SETDOCPOINTER, 0,
                         reinterpret_cast<sptr_t>(buf2show->getDocument()));
        _mapView->setCurrentBuffer(buf2show);

        // Sync folding state
        const std::vector<size_t>& lineStateVector = buf2show->getHeaderLineState(fromEditView);
        _mapView->syncFoldStateWith(lineStateVector);

        // Handle wrapping
        if (fromEditView->isWrap() && needToRecomputeWith(fromEditView)) {
            wrapMap(fromEditView);
        }

        // Restore map position if available
        MapPosition mp = buf2show->getMapPosition();
        if (mp.isValid()) {
            scrollMapWith(mp);
        }

        _isTemporarilyShowing = true;
    }
#else
    // TODO: Qt implementation
    (void)buf2show;
    (void)fromEditView;
#endif
}

void DocumentMap::setSyntaxHighlighting()
{
#ifndef NPP_LINUX
    if (_mapView) {
        Buffer* buf = _mapView->getCurrentBuffer();
        if (buf) {
            _mapView->defineDocType(buf->getLangType());
            _mapView->showMargin(ScintillaEditView::_SC_MARGE_FOLDER, false);
        }
    }
#else
    // TODO: Qt implementation
#endif
}

bool DocumentMap::needToRecomputeWith(const ScintillaEditView* editView)
{
    if (!_ppEditView || !*_ppEditView) return false;

    const ScintillaEditView* pEditView = editView ? editView : *_ppEditView;

    auto currentZoom = pEditView->execute(SCI_GETZOOM);
    if (_displayZoom != currentZoom) return true;

    int currentTextZoneWidth = pEditView->getTextZoneWidth();
    if (_displayWidth != currentTextZoneWidth) return true;

    return false;
}

void DocumentMap::initWrapMap()
{
    if (_mapView && _ppEditView && *_ppEditView) {
        // Resize map view to container
        QRect rect = this->rect();
        RECT rc = QRectToRECT(rect);
        _mapView->reSizeTo(rc);

        _mapView->wrap(false);
        _mapView->redraw(true);

        // Sync text direction
        bool isRTL = (*_ppEditView)->isTextDirectionRTL();
        if (_mapView->isTextDirectionRTL() != isRTL) {
            _mapView->changeTextDirection(isRTL);
        }
    }
}

void DocumentMap::changeTextDirection(bool isRTL)
{
    if (_mapView) {
        _mapView->changeTextDirection(isRTL);
    }
}

void DocumentMap::wrapMap(const ScintillaEditView* editView)
{
    if (!_mapView || !_ppEditView || !*_ppEditView) return;

    const ScintillaEditView* pEditView = editView ? editView : *_ppEditView;

    if (pEditView->isWrap()) {
        // Get current editor dimensions
        int editZoneWidth = pEditView->getTextZoneWidth();

        // Update tracking variables
        _displayWidth = editZoneWidth;
        _displayZoom = pEditView->execute(SCI_GETZOOM);

        // Calculate zoom ratio
        int zoomIndex = static_cast<int>(_displayZoom) + 10;
        if (zoomIndex < 0) zoomIndex = 0;
        if (zoomIndex >= static_cast<int>(sizeof(zoomRatio) / sizeof(zoomRatio[0]))) {
            zoomIndex = static_cast<int>(sizeof(zoomRatio) / sizeof(zoomRatio[0])) - 1;
        }
        double zr = zoomRatio[zoomIndex];

        // Compute document map width
        double docMapWidth = editZoneWidth / zr;

        // Resize map view
        QRect rect = this->rect();
        rect.setWidth(static_cast<int>(docMapWidth));
        RECT rc = QRectToRECT(rect);
        _mapView->reSizeTo(rc);

        _mapView->wrap(true);

        // Sync wrapping indent mode
        _mapView->execute(SCI_SETWRAPINDENTMODE,
                         pEditView->execute(SCI_GETWRAPINDENTMODE));

        // Note: Padding would be synced here if needed
        // This requires access to NppParameters which may not be available
        // in the Qt port yet
    }

    doMove();
}

void DocumentMap::scrollMap()
{
    if (!_mapView || !_ppEditView || !*_ppEditView) return;

    ScintillaEditView* pEditView = *_ppEditView;

    // Get the position of the 1st and last showing chars from the original edit view
    RECT rcEditView;
    pEditView->getClientRect(rcEditView);

    auto higherPos = pEditView->execute(SCI_POSITIONFROMPOINT, 0, 0);
    auto lowerPos = pEditView->execute(SCI_POSITIONFROMPOINT,
                                       rcEditView.right - rcEditView.left, rcEditView.bottom - rcEditView.top);

    // Let Scintilla scroll the map
    _mapView->execute(SCI_GOTOPOS, higherPos);
    _mapView->execute(SCI_GOTOPOS, lowerPos);

    // Get top position of highlight zone
    auto higherY = _mapView->execute(SCI_POINTYFROMPOSITION, 0, higherPos);

    // Get bottom position of highlight zone
    auto lowerY = static_cast<sptr_t>(0);
    auto lineHeightMapView = _mapView->execute(SCI_TEXTHEIGHT, 0);

    if (!pEditView->isWrap()) {
        // Not wrapped: mimic height of edit view
        auto lineHeightEditView = pEditView->execute(SCI_TEXTHEIGHT, 0);
        lowerY = higherY + lineHeightMapView * (rcEditView.bottom - rcEditView.top) / lineHeightEditView;
    } else {
        // Wrapped: ask Scintilla, since in the map view the current range
        // of edit view might be wrapped differently
        lowerY = _mapView->execute(SCI_POINTYFROMPOSITION, 0, lowerPos) + lineHeightMapView;
    }

    // Update view zone highlight
    if (_viewZone) {
        _viewZone->setZone(static_cast<int>(higherY), static_cast<int>(lowerY));
    }
}

void DocumentMap::scrollMapWith(const MapPosition& mapPos)
{
    if (!_mapView) return;

    // Visible document line for the map view
    auto firstVisibleDisplayLineMap = _mapView->execute(SCI_GETFIRSTVISIBLELINE);
    auto firstVisibleDocLineMap = _mapView->execute(SCI_DOCLINEFROMVISIBLE, firstVisibleDisplayLineMap);
    auto nbLineMap = _mapView->execute(SCI_LINESONSCREEN, firstVisibleDocLineMap);
    auto lastVisibleDocLineMap = _mapView->execute(SCI_DOCLINEFROMVISIBLE,
                                                   firstVisibleDisplayLineMap + nbLineMap);

    // If part of editor view is out of map, then scroll map
    sptr_t mapLineToScroll = 0;
    if (lastVisibleDocLineMap < mapPos._lastVisibleDocLine) {
        mapLineToScroll = mapPos._lastVisibleDocLine;
    } else {
        mapLineToScroll = mapPos._firstVisibleDocLine;
    }

    // Scroll to make whole view zone visible
    _mapView->execute(SCI_GOTOLINE, mapLineToScroll);

    // Get the editor's higher/lower Y, then compute the map's higher/lower Y
    sptr_t higherY = 0;
    sptr_t lowerY = 0;

    if (!mapPos._isWrap) {
        auto higherPos = _mapView->execute(SCI_POSITIONFROMLINE, mapPos._firstVisibleDocLine);
        auto lowerPos = _mapView->execute(SCI_POSITIONFROMLINE, mapPos._lastVisibleDocLine);
        higherY = _mapView->execute(SCI_POINTYFROMPOSITION, 0, higherPos);
        lowerY = _mapView->execute(SCI_POINTYFROMPOSITION, 0, lowerPos);
        if (lowerY == 0) {
            auto lineHeight = _mapView->execute(SCI_TEXTHEIGHT, mapPos._firstVisibleDocLine);
            lowerY = mapPos._nbLine * lineHeight + mapPos._firstVisibleDocLine;
        }
    } else {
        higherY = _mapView->execute(SCI_POINTYFROMPOSITION, 0, mapPos._higherPos);
        auto lineHeight = _mapView->execute(SCI_TEXTHEIGHT, mapPos._firstVisibleDocLine);
        lowerY = mapPos._nbLine * lineHeight + higherY;
    }

    // Update view zone highlight
    if (_viewZone) {
        _viewZone->setZone(static_cast<int>(higherY), static_cast<int>(lowerY));
    }
}

void DocumentMap::doMove()
{
    // Position the view zone widget over the map view
    if (_viewZone && _mapContainer) {
        _viewZone->setGeometry(_mapContainer->rect());
    }
}

void DocumentMap::fold(size_t line, bool foldOrNot)
{
    if (_mapView) {
        _mapView->fold(line, foldOrNot, false);
    }
}

void DocumentMap::foldAll(bool mode)
{
    if (_mapView) {
        _mapView->foldAll(mode);
    }
}

void DocumentMap::scrollMap(bool direction, bool perPage)
{
    if (!_ppEditView || !*_ppEditView) return;

    ScintillaEditView* pEditView = *_ppEditView;

    // Visible line for the code view
    auto firstVisibleDisplayLine = pEditView->execute(SCI_GETFIRSTVISIBLELINE);
    auto nbLine = pEditView->execute(SCI_LINESONSCREEN, firstVisibleDisplayLine);
    auto nbLine2go = perPage ? nbLine : 1;

    pEditView->execute(SCI_LINESCROLL, 0, direction ? nbLine2go : -nbLine2go);

    scrollMap();
}

void DocumentMap::redrawMap(bool forceUpdate)
{
    if (_mapView) {
        _mapView->execute(SCI_COLOURISE, 0, -1);
    }
    update();
}

void DocumentMap::updateMap()
{
    if (_updating) return;
    _updating = true;

    scrollMap();

    _updating = false;
}

void DocumentMap::setZoomLevel(int level)
{
    _zoomLevel = level;

    if (_zoomSlider) {
        _zoomSlider->setValue(level);
    }

    if (_zoomLabel) {
        _zoomLabel->setText(QString::number(level));
    }

    if (_mapView) {
        _mapView->execute(SCI_SETZOOM, level);
    }
}

void DocumentMap::onMapClicked(int y)
{
    if (!_mapView || !_ppEditView || !*_ppEditView || !_viewZone) return;

    int currentCenterPosY = _viewZone->getCurrentCenterPosY();
    auto pixelPerLine = _mapView->execute(SCI_TEXTHEIGHT, 0);
    int jumpDistance = y - currentCenterPosY;
    auto nbLine2jump = jumpDistance / pixelPerLine;

    (*_ppEditView)->execute(SCI_LINESCROLL, 0, nbLine2jump);
    scrollMap();
}

void DocumentMap::onMapScrolled()
{
    // Handle continuous scrolling while dragging
    // This is handled by the zoneDragged signal which calls onMapClicked
}

void DocumentMap::onZoomChanged(int value)
{
    setZoomLevel(value);

    // Recompute wrapping if needed
    if (_ppEditView && *_ppEditView && (*_ppEditView)->isWrap()) {
        wrapMap();
    }
}

void DocumentMap::onMainEditorScrolled()
{
    if (!_updating) {
        updateMap();
    }
}

void DocumentMap::onMainEditorChanged()
{
    reloadMap();
}

void DocumentMap::syncWithEditor()
{
    highlightVisibleRegion();
}

void DocumentMap::highlightVisibleRegion()
{
    scrollMap();
}

void DocumentMap::scrollEditorToPosition(int pos)
{
    if (_ppEditView && *_ppEditView) {
        (*_ppEditView)->execute(SCI_GOTOPOS, pos);
        scrollMap();
    }
}

void DocumentMap::resizeEvent(QResizeEvent* event)
{
    StaticDialog::resizeEvent(event);

    if (_mapView) {
        // Resize the map view to fit the container
        QRect rect = this->rect();
        RECT rc = QRectToRECT(rect);
        _mapView->reSizeTo(rc);
    }

    doMove();
}

} // namespace QtControls
