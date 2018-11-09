#include "slacktext.h"
#include "slacktext_p.h"
#include <QTextBoundaryFinder>

#include "QtQuick/private/qquicktextnode_p.h"
#include "QtQuick/private/qquicktextdocument_p.h"

#include <QtQuick/qsgsimplerectnode.h>
#include <private/qv4scopedvalue_p.h>
#include <QtQuickTemplates2/private/qquicklabel_p_p.h>

qreal alignedX(const qreal textWidth, const qreal itemWidth, int alignment)
{
    qreal x = 0;
    switch (alignment) {
    case Qt::AlignLeft:
    case Qt::AlignJustify:
        break;
    case Qt::AlignRight:
        x = itemWidth - textWidth;
        break;
    case Qt::AlignHCenter:
        x = (itemWidth - textWidth) / 2;
        break;
    }
    return x;
}

qreal alignedY(const qreal textHeight, const qreal itemHeight, int alignment)
{
    qreal y = 0;
    switch (alignment) {
    case Qt::AlignTop:
        break;
    case Qt::AlignBottom:
        y = itemHeight - textHeight;
        break;
    case Qt::AlignVCenter:
        y = (itemHeight - textHeight) / 2;
        break;
    }
    return y;
}

SlackText::SlackText(QQuickItem* parent)
    : QQuickLabel(parent)
{
    d_ptr = new SlackTextPrivate;
    d_ptr->q_ptr = this;
    Q_D(SlackText);
    d->init();
}

void SlackTextPrivate::prepareText() {
    if (m_dirty && m_tp->extra.isAllocated() && m_tp->extra->doc) {
        m_modified = false;
        const QPalette& palette = QGuiApplication::palette();
        bool singleQuote = false;
        QTextCursor prevCursor(m_tp->extra->doc);
        while (!prevCursor.isNull() && !prevCursor.atEnd()) {
            QString searchQuote = "```";
            prevCursor = m_tp->extra->doc->find(searchQuote, prevCursor);
            if (prevCursor.isNull()) {
                prevCursor = QTextCursor(m_tp->extra->doc);
                searchQuote = "`";
                singleQuote = true;
                prevCursor = m_tp->extra->doc->find(searchQuote, prevCursor);
            }
            if (!prevCursor.isNull()) {
                QTextCursor nextCursor = m_tp->extra->doc->find(searchQuote, prevCursor);
                if (nextCursor.isNull()) {
                    qWarning() << "no next cursor found! Assume its will be end of the document";
                    nextCursor = m_tp->extra->doc->rootFrame()->lastCursorPosition();
                }
                m_modified = true;
                bool isundo = m_tp->extra->doc->isUndoRedoEnabled();
                m_tp->extra->doc->setUndoRedoEnabled(false);
                //nextCursor.beginEditBlock();
                prevCursor.movePosition(QTextCursor::NextCharacter,
                                        QTextCursor::KeepAnchor,
                                        nextCursor.position() - prevCursor.position());
                QString selectedText = prevCursor.selectedText();
                //qDebug() << "found quote" << selectedText << singleQuote;
                selectedText = selectedText.remove(searchQuote);
                prevCursor.removeSelectedText();

                if (singleQuote) {
                    QTextCharFormat chFmt = prevCursor.charFormat();
                    QFont fnt = chFmt.font();
                    const QFontMetrics fm(fnt);
                    const QRectF strRect = fm.boundingRect(selectedText);
                    //fmt.setPosition(QTextFrameFormat::InFlow);
                    //fmt.setWidth(QTextLength(QTextLength::FixedLength, strRect.width()));
                    //fmt.setHeight(QTextLength(QTextLength::FixedLength, strRect.height()));

                    //make some extra space for better visibility
                    selectedText.prepend(" ");
                    selectedText += " ";
                    chFmt.setBackground(QBrush(palette.color(QPalette::AlternateBase)));
                    chFmt.setForeground(QBrush(palette.color(QPalette::HighlightedText)));
                    prevCursor.insertText(selectedText, chFmt);
                } else {
                    QTextFrameFormat fmt;
                    fmt.setPageBreakPolicy(QTextBlockFormat::PageBreak_AlwaysBefore|
                                           QTextBlockFormat::PageBreak_AlwaysAfter);
                    fmt.setBorderStyle(QTextFrameFormat::BorderStyle_Dotted);
                    fmt.setBorder(singleQuote ? 1 : 2);
                    fmt.setPadding(singleQuote ? 1: 5);
                    fmt.setBackground(QBrush(palette.color(QPalette::AlternateBase)));
                    fmt.setForeground(QBrush(palette.color(QPalette::HighlightedText)));
                    QTextFrame *codeBlockFrame = prevCursor.insertFrame(fmt);
                    QTextCursor blockCursor = codeBlockFrame->firstCursorPosition();
                    blockCursor.insertText(selectedText);
                }

                //qDebug() << "frame" << d->m_tp->extra->doc->rootFrame()->frameFormat().width().type();//codeBlockFrame->firstPosition() << codeBlockFrame->lastPosition();
                //nextCursor.endEditBlock();
                m_tp->extra->doc->setUndoRedoEnabled(isundo);
            }
        }
        if (m_modified) {
            qDebug() << "updating";
            m_lp->updateSize();
            m_lp->updateLayout();
            m_modified = false;
        }
        m_dirty = false;
    }
}

void SlackText::componentComplete()
{
    Q_D(SlackText);

    QQuickLabel::componentComplete();
    d->prepareText();
    //qDebug() << "text is rich" << d->m_tp->richText << d->m_tp->extra.isAllocated() << d->m_tp->extra->doc << d->m_tp->updateType;
    // create frames for quotes
    // TODO: code highlight?


    //QTextCursor someCursor(d->m_tp->extra->doc);
    //qDebug() << "text is rich: " << someCursor.block().text();
    //        QList<QTextFrame *> frames;
    //        frames.append(d->m_tp->extra->doc->rootFrame());
    //        while (!frames.isEmpty()) {
    //            QTextFrame *textFrame = frames.takeFirst();
    //            qWarning() << "frame" << textFrame;
    //            frames.append(textFrame->childFrames());
    //            QTextCursor cursor = textFrame->firstCursorPosition();
    //            QTextFrameFormat fmt = textFrame->frameFormat();
    //            qDebug() << "text" << cursor.block().text() << fmt.padding() << fmt.height().value(100);
    //            fmt.setBackground(QBrush(QColor(Qt::red)));
    //            //fmt.setTopMargin(20);
    //            textFrame->setFrameFormat(fmt);
    //        }
    //        for (QTextFrame *textFrame : d->m_tp->extra->doc->rootFrame()->childFrames()) {
    //            QTextCursor cursor = textFrame->firstCursorPosition();
    //            QTextFrameFormat fmt = textFrame->frameFormat();
    //            qDebug() << "text" << cursor.block().text() << fmt.padding();
    //            fmt.setBackground(QBrush(QColor(Qt::red)));
    //            //fmt.setTopMargin(20);
    //            textFrame->setFrameFormat(fmt);
    //        }


}


QColor SlackText::selectionColor() const
{
    Q_D(const SlackText);
    return d->selectionColor;
}

void SlackText::setSelectionColor(const QColor &color)
{
    Q_D(SlackText);
    if (d->selectionColor == color)
        return;

    d->selectionColor = color;
    if (d->hasSelectedText()) {
        d->textLayoutDirty = true;
        d->m_lp->updateType = QQuickTextPrivate::UpdatePaintNode;
        polish();
        update();
    }
    emit selectionColorChanged();
}
/*!
    \qmlproperty color QtQuick::TextInput::selectedTextColor

    The highlighted text color, used in selections.
*/
QColor SlackText::selectedTextColor() const
{
    Q_D(const SlackText);
    return d->selectedTextColor;
}

void SlackText::setSelectedTextColor(const QColor &color)
{
    Q_D(SlackText);
    if (d->selectedTextColor == color)
        return;

    d->selectedTextColor = color;
    if (d->hasSelectedText()) {
        d->textLayoutDirty = true;
        d->m_lp->updateType = QQuickTextPrivate::UpdatePaintNode;
        polish();
        update();
    }
    emit selectedTextColorChanged();
}

int SlackText::selectionStart() const
{
    Q_D(const SlackText);
    return d->lastSelectionStart;
}

int SlackText::selectionEnd() const
{
    Q_D(const SlackText);
    return d->lastSelectionEnd;
}

void SlackText::select(int start, int end)
{
    Q_D(SlackText);
    if (start < 0 || end < 0 || start > text().length() || end > text().length())
        return;
    d->setSelection(start, end-start);
}

QString SlackText::selectedText() const
{
    Q_D(const SlackText);
    return d->selectedText();
}

void SlackTextPrivate::init()
{
    Q_Q(SlackText);

    m_lp = QQuickLabelPrivate::get(qobject_cast<QQuickLabel *>(q_ptr));
    m_tp = QQuickTextPrivate::get(qobject_cast<QQuickText *>(q_ptr));

#if QT_CONFIG(clipboard)
    if (QGuiApplication::clipboard()->supportsSelection())
        q->setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton);
    else
#endif
        q->setAcceptedMouseButtons(Qt::LeftButton);

    lastSelectionStart = 0;
    lastSelectionEnd = 0;
}

int SlackTextPrivate::findInMask(int pos, bool forward, bool findSeparator, QChar searchChar) const
{
    if (pos >= m_maxLength || pos < 0)
        return -1;

    int end = forward ? m_maxLength : -1;
    int step = forward ? 1 : -1;
    int i = pos;

    while (i != end) {
        if (findSeparator) {
            if (m_maskData[i].separator && m_maskData[i].maskChar == searchChar)
                return i;
        } else {
            if (!m_maskData[i].separator) {
                return i;
            }
        }
        i += step;
    }
    return -1;
}

void SlackTextPrivate::moveSelectionCursor(int pos, bool mark)
{
    Q_Q(SlackText);

    if (pos != m_cursor) {
        separate();
        if (m_maskData)
            pos = pos > m_cursor ? nextMaskBlank(pos) : prevMaskBlank(pos);
    }
    if (mark) {
        int anchor;
        if (m_selend > m_selstart && m_cursor == m_selstart)
            anchor = m_selend;
        else if (m_selend > m_selstart && m_cursor == m_selend)
            anchor = m_selstart;
        else
            anchor = m_cursor;
        m_selstart = qMin(anchor, pos);
        m_selend = qMax(anchor, pos);
    } else {
        internalDeselect();
    }
    m_cursor = pos;
    if (mark || m_selDirty) {
        m_selDirty = false;
        emit q->selectionChanged();
    }
    //emitCursorPositionChanged();

}

QRectF SlackText::positionToRectangle(int pos)
{
    Q_D(SlackText);
    //    if (d->m_echoMode == NoEcho)
    //        pos = 0;
    //#if QT_CONFIG(im)
    //    else if (pos > d->m_cursor)
    //        pos += d->preeditAreaText().length();
    //#endif
    QTextLine l = d->m_lp->layout.lineForTextPosition(pos);
    //QTextLine l = d->layout.lineForTextPosition(pos);
    if (!l.isValid())
        return QRectF();
    qreal x = l.cursorToX(pos)/* - d->hscroll*/;
    qreal y = l.y()/* - d->vscroll*/;
    qreal w = 1;
    //    if (d->overwriteMode) {
    //        if (pos < text().length())
    //            w = l.cursorToX(pos + 1) - x;
    //        else
    //            w = QFontMetrics(font()).width(QLatin1Char(' ')); // in sync with QTextLine::draw()
    //    }
    return QRectF(x, y, w, l.height());
}

void SlackText::positionAt(QQmlV4Function *args)
{
    Q_D(SlackText);

    qreal x = 0;
    qreal y = 0;
    QTextLine::CursorPosition position = QTextLine::CursorBetweenCharacters;

    if (args->length() < 1)
        return;

    int i = 0;
    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue arg(scope, (*args)[0]);
    x = arg->toNumber();

    if (++i < args->length()) {
        arg = (*args)[i];
        y = arg->toNumber();
    }

    if (++i < args->length()) {
        arg = (*args)[i];
        position = QTextLine::CursorPosition(arg->toInt32());
    }

    int pos = d->positionAt(x, y, position);
    const int cursor = d->m_cursor;
    if (pos > cursor) {
#if QT_CONFIG(im)
        const int preeditLength = 0;
        pos = pos > cursor + preeditLength
                ? pos - preeditLength
                : cursor;
#else
        pos = cursor;
#endif
    }
    args->setReturnValue(QV4::Encode(pos));
}

int SlackTextPrivate::positionAt(qreal x, qreal y, QTextLine::CursorPosition position)
{
    Q_Q(SlackText);
    int pos = 0;

    if (m_tp->richText && m_tp->extra.isAllocated() && m_tp->extra->doc) {
        //qDebug() << __PRETTY_FUNCTION__ << y << m_tp->layedOutTextRect.height() << m_tp->availableHeight() << m_tp->extra->topPadding;
        QPointF translatedMousePos = QPointF(x, y);
        translatedMousePos.rx() -= alignedX(m_tp->layedOutTextRect.width(), m_tp->availableWidth(), q->effectiveHAlign());
        translatedMousePos.ry() -= m_tp->extra->topPadding;
        //qDebug() << "corrected mouse pos" << translatedMousePos;
        pos = m_tp->extra->doc->documentLayout()->hitTest(translatedMousePos, Qt::FuzzyHit);
    } else {
        x += q->leftPadding();
        y += q->topPadding();
        QTextLine line = m_lp->layout.lineAt(0);
        for (int i = 1; i < m_lp->layout.lineCount(); ++i) {
            QTextLine nextLine = m_lp->layout.lineAt(i);

            if (y < (line.rect().bottom() + nextLine.y()) / 2)
                break;
            line = nextLine;
        }
        pos = line.isValid() ? line.xToCursor(x, position) : 0;
    }
    return pos;

}

void SlackText::invalidateFontCaches()
{
    QQuickText::invalidateFontCaches();
}

void SlackText::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(SlackText);

    if (d->selectByMouse && event->button() == Qt::LeftButton) {
        //#if QT_CONFIG(im)
        //        d->commitPreedit();
        //#endif
        int cursor = d->positionAt(event->localPos());
        qDebug() << __PRETTY_FUNCTION__ << cursor << event->localPos() << event->pos() << event->windowPos();
        d->selectWordAtPos(cursor);
        event->setAccepted(true);
        if (!d->hasPendingTripleClick()) {
            d->tripleClickStartPoint = event->localPos();
            d->tripleClickTimer.start();
        }
    } else {
        //        if (d->sendMouseEventToInputContext(event))
        //            return;
        QQuickLabel::mouseDoubleClickEvent(event);
    }
}


/*!
    \internal

    Sets the selection to cover the word at the given cursor position.
    The word boundaries are defined by the behavior of QTextLayout::SkipWords
    cursor mode.
*/
void SlackTextPrivate::selectWordAtPos(int cursor)
{
    Q_Q(SlackText);
    int next = cursor + 1;
    if (next > end())
        --next;

    int c = 0;
    int end = 0;
    if (m_lp->richText) {
        QTextDocumentPrivate* td_p = m_tp->extra->doc->docHandle();
        c = td_p->previousCursorPosition(next, QTextLayout::SkipWords);
        end = td_p->nextCursorPosition(c, QTextLayout::SkipWords);
        qDebug() << cursor << next << c << end;
    } else {
        c = m_lp->layout.previousCursorPosition(next, QTextLayout::SkipWords);
        end = m_lp->layout.nextCursorPosition(c, QTextLayout::SkipWords);
    }
    moveSelectionCursor(c, false);
    // ## text layout should support end of words.
    while (end > cursor && q->text()[end-1].isSpace())
        --end;
    moveSelectionCursor(end, true);
}


#if QT_CONFIG(clipboard)
/*!
    \internal

    Copies the currently selected text into the clipboard using the given
    \a mode.

    \note If the echo mode is set to a mode other than Normal then copy
    will not work.  This is to prevent using copy as a method of bypassing
    password features of the line control.
*/
void SlackTextPrivate::copy(QClipboard::Mode mode) const
{
    QString t = selectedText();
    if (!t.isEmpty()) {
        qDebug() << "copy to clipboard:" << t;
        QGuiApplication::clipboard()->setText(t, mode);
    }
}

#endif // clipboard

/*!
    \qmlmethod QtQuick::TextInput::copy()

    Copies the currently selected text to the system clipboard.

    \note If the echo mode is set to a mode other than Normal then copy
    will not work.  This is to prevent using copy as a method of bypassing
    password features of the line control.
*/
void SlackText::copy()
{
    Q_D(SlackText);
    d->copy();
}

void SlackText::mousePressEvent(QMouseEvent *event)
{
    Q_D(SlackText);

    d->pressPos = event->localPos();

    //    if (d->sendMouseEventToInputContext(event))
    //        return;

    if (d->selectByMouse) {
        setKeepMouseGrab(false);
        d->selectPressed = true;
        QPointF distanceVector = d->pressPos - d->tripleClickStartPoint;
        if (d->hasPendingTripleClick()
                && distanceVector.manhattanLength() < QGuiApplication::styleHints()->startDragDistance()) {
            event->setAccepted(true);
            selectAll();
            return;
        }
    }

    bool mark = (event->modifiers() & Qt::ShiftModifier) && d->selectByMouse;
    int cursor = d->positionAt(event->localPos());
    d->moveSelectionCursor(cursor, mark);

    //    if (d->focusOnPress && !qGuiApp->styleHints()->setFocusOnTouchRelease())
    //        ensureActiveFocus();

    QQuickLabel::mousePressEvent(event);
    event->setAccepted(true);
}

void SlackText::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(SlackText);

    if (d->selectPressed) {
        if (qAbs(int(event->localPos().x() - d->pressPos.x())) > QGuiApplication::styleHints()->startDragDistance())
            setKeepMouseGrab(true);
        moveCursorSelection(d->positionAt(event->localPos()), d->mouseSelectionMode);
        event->setAccepted(true);
    } else {
        QQuickLabel::mouseMoveEvent(event);
    }
}

void SlackText::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(SlackText);
    //    if (d->sendMouseEventToInputContext(event))
    //        return;
    if (keepMouseGrab() == false && d->hasSelectedText() == false) {
        QString link = d->m_tp->anchorAt(event->localPos());
        if (!link.isEmpty()) {
            qDebug() << "activating link:" << link << text() << d->m_selstart << d->m_selend;
            emit linkActivated(d->m_tp->extra->activeLink);
            event->setAccepted(true);
            return;
        }
    } else if (d->selectPressed) {
        d->selectPressed = false;
        setKeepMouseGrab(false);
    }
#if QT_CONFIG(clipboard)
    if (QGuiApplication::clipboard()->supportsSelection()) {
        if (event->button() == Qt::LeftButton) {
            d->copy(QClipboard::Selection);
        } else if (event->button() == Qt::MidButton) {
            d->deselect();
        }
    }
#endif

    //    if (d->focusOnPress && qGuiApp->styleHints()->setFocusOnTouchRelease())
    //        ensureActiveFocus();

    if (!event->isAccepted())
        QQuickLabel::mouseReleaseEvent(event);
}

void SlackText::mouseUngrabEvent()
{
    Q_D(SlackText);
    d->selectPressed = false;
    setKeepMouseGrab(false);
}

void SlackText::hoverEnterEvent(QHoverEvent *event)
{
    //qDebug() << __PRETTY_FUNCTION__ << event->pos();
    QQuickLabel::hoverEnterEvent(event);
}

void SlackText::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(SlackText);
    //qDebug() << __PRETTY_FUNCTION__ << event->pos();
    //
    QString link;
    QString imglink;
    QPointF translatedMousePos = event->posF();
    translatedMousePos.rx() -= leftPadding();
    translatedMousePos.ry() -= topPadding() + alignedY(d->m_tp->layedOutTextRect.height() + d->m_tp->lineHeightOffset(),
                                                       d->m_tp->availableHeight(), d->m_tp->vAlign);
    if (d->m_tp->styledText) {
        QString link = d->m_tp->anchorAt(&d->m_tp->layout, translatedMousePos);
        if (link.isEmpty() && d->m_tp->elideLayout)
            link = d->m_tp->anchorAt(d->m_tp->elideLayout, translatedMousePos);
    } else if (d->m_tp->richText && d->m_tp->extra.isAllocated() && d->m_tp->extra->doc) {
        translatedMousePos.rx() -= alignedX(d->m_tp->layedOutTextRect.width(), d->m_tp->availableWidth(),
                                            effectiveHAlign());
        link = d->m_tp->extra->doc->documentLayout()->anchorAt(translatedMousePos);
        imglink = d->m_tp->extra->doc->documentLayout()->imageAt(translatedMousePos);
    }
    if (link != d->m_linkHovered) {
        //qDebug() << link << d->m_linkHovered;
        d->m_linkHovered = link;
        emit linkHovered(link);
    }
    if (imglink != d->m_imageHovered) {
        d->m_imageHovered = imglink;
        if (!imglink.isEmpty()) {
            emit imageHovered(imglink);
        }
    }
    QQuickLabel::hoverMoveEvent(event);
}

void SlackText::hoverLeaveEvent(QHoverEvent *event)
{
    //qDebug() << __PRETTY_FUNCTION__ << event->pos();
    QQuickLabel::hoverLeaveEvent(event);
}

/**
 * @brief SlackText::updatePaintNode. Direct copy of QQuickText one with select modifications. Keep in sync
 * @param oldNode
 * @param data
 * @return
 */

QSGNode *SlackText::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);
    Q_D(SlackText);

    if (d->m_lp->text.isEmpty()) {
        delete oldNode;
        return nullptr;
    }

    if (d->m_lp->updateType != QQuickTextPrivate::UpdatePaintNode && oldNode != nullptr) {
        // Update done in preprocess() in the nodes
        d->m_lp->updateType = QQuickTextPrivate::UpdateNone;
        return oldNode;
    }

    d->m_tp->updateType = QQuickTextPrivate::UpdateNone;

    const qreal dy = alignedY(d->m_lp->layedOutTextRect.height() + d->m_lp->lineHeightOffset(), d->m_lp->availableHeight(), d->m_lp->vAlign) + topPadding();

    //qDebug() << dy << topPadding() << d->m_lp->vAlign;

    QQuickTextNode *node = nullptr;
    if (!oldNode)
        node = new QQuickTextNode(this);
    else
        node = static_cast<QQuickTextNode *>(oldNode);

    node->setUseNativeRenderer(d->m_lp->renderType == NativeRendering);
    node->deleteContent();
    node->setMatrix(QMatrix4x4());

    const QColor color = QColor::fromRgba(d->m_lp->color);
    const QColor styleColor = QColor::fromRgba(d->m_lp->styleColor);
    const QColor linkColor = QColor::fromRgba(d->m_lp->linkColor);

    //qDebug() << d->m_lp->richText << d->m_lp->layout.text() << d->selectionStart() << d->selectionEnd() << d->selectionColor << d->selectedTextColor;
    if (d->m_lp->richText) {
        const qreal dx = alignedX(d->m_lp->layedOutTextRect.width(), d->m_lp->availableWidth(), effectiveHAlign()) + leftPadding();
        d->m_lp->ensureDoc();

        //        QTextBlock currentBlock = d->m_tp->extra->doc->begin();

        //        while (currentBlock.isValid()) {
        //            QTextBlock::iterator it;
        //            for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
        //                QTextFragment currentFragment = it.fragment();
        //                if (currentFragment.isValid()) {
        //                    QTextImageFormat newImageFormat = currentFragment.charFormat().toImageFormat();
        //                    if (newImageFormat.isValid()) {
        //                        //currentFragment.charFormat().setVerticalAlignment(QTextCharFormat::AlignBaseline);
        //                        qDebug() << newImageFormat.name() << newImageFormat.verticalAlignment() << d->m_tp->extra->doc->rootFrame()->childFrames().count();
        //                    }
        //                }
        //            }
        //            currentBlock = currentBlock.next();
        //        }

        node->addTextDocument(QPointF(dx, dy), d->m_tp->extra->doc, color, d->m_lp->style, styleColor, linkColor,
                              d->selectionColor, d->selectedTextColor,
                              d->selectionStart(), d->selectionEnd() - 1);
    } else if (d->m_lp->layedOutTextRect.width() > 0) {
        const qreal dx = alignedX(d->m_lp->lineWidth, d->m_lp->availableWidth(), effectiveHAlign()) + leftPadding();
        node->addRectangleNode(QRectF(dx, dy, d->m_lp->layedOutTextRect.width(), d->m_lp->layedOutTextRect.height()),
                               QColor(Qt::gray));
        int unelidedLineCount = d->m_lp->lineCount;
        if (d->m_lp->elideLayout)
            unelidedLineCount -= 1;
        if (unelidedLineCount > 0) {
            node->addTextLayout(
                        QPointF(dx, dy),
                        &d->m_lp->layout,
                        color, d->m_lp->style, styleColor, linkColor,
                        d->selectionColor, d->selectedTextColor,
                        selectionStart(), d->selectionEnd() - 1,
                        0, unelidedLineCount);
        }
        if (d->m_lp->elideLayout)
            node->addTextLayout(QPointF(dx, dy), d->m_lp->elideLayout, color, d->m_lp->style, styleColor, linkColor,
                                d->selectionColor, d->selectedTextColor,
                                selectionStart(), d->selectionEnd() - 1);

        if (d->m_lp->extra.isAllocated()) {
            for (QQuickStyledTextImgTag *img : qAsConst(d->m_tp->extra->visibleImgTags)) {
                QQuickPixmap *pix = img->pix;
                if (pix && pix->isReady()) {
                    qDebug() << "image" << img->url << img->pos << pix->rect();
                    node->addImage(QRectF(img->pos.x() + dx, img->pos.y() + dy, pix->width(), pix->height()), pix->image());
                }
            }
        }
    }

    // The font caches have now been initialized on the render thread, so they have to be
    // invalidated before we can use them from the main thread again.
    invalidateFontCaches();

    return node;

}

void SlackText::deselect()
{
    Q_D(SlackText);
    d->deselect();
}

void SlackText::selectAll()
{
    Q_D(SlackText);
    d->setSelection(0, text().length());
}

void SlackText::selectWord()
{
    Q_D(SlackText);
    d->selectWordAtPos(d->m_cursor);
}

bool SlackText::selectByMouse() const
{
    Q_D(const SlackText);
    return d->selectByMouse;
}

void SlackText::setSelectByMouse(bool on)
{
    Q_D(SlackText);
    if (d->selectByMouse != on) {
        d->selectByMouse = on;
        emit selectByMouseChanged(on);
    }
}

SlackText::SelectionMode SlackText::mouseSelectionMode() const
{
    Q_D(const SlackText);
    return d->mouseSelectionMode;
}

void SlackText::setMouseSelectionMode(SelectionMode mode)
{
    Q_D(SlackText);
    if (d->mouseSelectionMode != mode) {
        d->mouseSelectionMode = mode;
        emit mouseSelectionModeChanged(mode);
    }
}

bool SlackText::persistentSelection() const
{
    Q_D(const SlackText);
    return d->persistentSelection;
}

void SlackText::setTextFormat(TextFormat format)
{
    QQuickLabel::setTextFormat(format);
}

//qreal SlackText::topPadding() const
//{
//    return QQuickLabel::topPadding();
//}

//void SlackText::setTopPadding(qreal padding)
//{
//    QQuickLabel::setTopPadding(padding);
//}

//void SlackText::resetTopPadding()
//{
//    QQuickLabel::resetTopPadding();
//}

QQuickText::TextFormat SlackText::textFormat() const
{
    return QQuickLabel::textFormat();
}

QString SlackText::text() const
{
    return QQuickLabel::text();
}

void SlackText::setText(const QString &txt)
{
    Q_D(SlackText);
    QQuickLabel::setText(txt);
    d->m_dirty = true;
    d->prepareText();
}

QString SlackText::hoveredLink() const
{
    Q_D(const SlackText);
    return d->m_linkHovered;
}

QString SlackText::hoveredImage() const
{
    Q_D(const SlackText);
    return d->m_imageHovered;
}

void SlackText::setPersistentSelection(bool on)
{
    Q_D(SlackText);
    if (d->persistentSelection == on)
        return;
    d->persistentSelection = on;
    emit persistentSelectionChanged();
}

void SlackText::moveCursorSelection(int position)
{
    Q_D(SlackText);
    d->moveSelectionCursor(position, true);
}

void SlackText::moveCursorSelection(int pos, SelectionMode mode)
{
    Q_D(SlackText);

    //qDebug() << pos << mode << d->m_cursor;
    if (mode == SelectCharacters) {
        d->moveSelectionCursor(pos, true);
    } else if (pos != d->m_cursor){
        const int cursor = d->m_cursor;
        int anchor;
        if (!d->hasSelectedText())
            anchor = d->m_cursor;
        else if (d->selectionStart() == d->m_cursor)
            anchor = d->selectionEnd();
        else
            anchor = d->selectionStart();

        if (anchor < pos || (anchor == pos && cursor < pos)) {
            const QString text = this->text();
            QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
            finder.setPosition(anchor);

            const QTextBoundaryFinder::BoundaryReasons reasons = finder.boundaryReasons();
            if (anchor < text.length() && (reasons == QTextBoundaryFinder::NotAtBoundary
                                           || (reasons & QTextBoundaryFinder::EndOfItem))) {
                finder.toPreviousBoundary();
            }
            anchor = finder.position() != -1 ? finder.position() : 0;

            finder.setPosition(pos);
            if (pos > 0 && !finder.boundaryReasons())
                finder.toNextBoundary();
            const int cursor = finder.position() != -1 ? finder.position() : text.length();

            d->setSelection(anchor, cursor - anchor);
        } else if (anchor > pos || (anchor == pos && cursor > pos)) {
            const QString text = this->text();
            QTextBoundaryFinder finder(QTextBoundaryFinder::Word, text);
            finder.setPosition(anchor);

            const QTextBoundaryFinder::BoundaryReasons reasons = finder.boundaryReasons();
            if (anchor > 0 && (reasons == QTextBoundaryFinder::NotAtBoundary
                               || (reasons & QTextBoundaryFinder::StartOfItem))) {
                finder.toNextBoundary();
            }
            anchor = finder.position() != -1 ? finder.position() : text.length();

            finder.setPosition(pos);
            if (pos < text.length() && !finder.boundaryReasons())
                finder.toPreviousBoundary();
            const int cursor = finder.position() != -1 ? finder.position() : 0;

            d->setSelection(anchor, cursor - anchor);
        }
    }
}

void SlackText::selectionChanged()
{
    Q_D(SlackText);
    d->textLayoutDirty = true; //TODO: Only update rect in selection
    d->m_lp->updateType = QQuickTextPrivate::UpdatePaintNode;
    polish();
    update();
    emit selectedTextChanged();

    if (d->lastSelectionStart != d->selectionStart()) {
        d->lastSelectionStart = d->selectionStart();
        if (d->lastSelectionStart == -1)
            d->lastSelectionStart = d->m_cursor;
        emit selectionStartChanged();
    }
    if (d->lastSelectionEnd != d->selectionEnd()) {
        d->lastSelectionEnd = d->selectionEnd();
        if (d->lastSelectionEnd == -1)
            d->lastSelectionEnd = d->m_cursor;
        emit selectionEndChanged();
    }
}

void SlackTextPrivate::setSelection(int start, int length)
{
    Q_Q(SlackText);

    if (start < 0 || start > q->text().length()) {
        qWarning("SlackTextPrivate::setSelection: Invalid start position");
        return;
    }

    if (length > 0) {
        if (start == m_selstart && start + length == m_selend && m_cursor == m_selend)
            return;
        m_selstart = start;
        m_selend = qMin(start + length, q->text().length());
        m_cursor = m_selend;
    } else if (length < 0){
        if (start == m_selend && start + length == m_selstart && m_cursor == m_selstart)
            return;
        m_selstart = qMax(start + length, 0);
        m_selend = start;
        m_cursor = m_selstart;
    } else if (m_selstart != m_selend) {
        m_selstart = 0;
        m_selend = 0;
        m_cursor = start;
    } else {
        m_cursor = start;
        //emitCursorPositionChanged();
        return;
    }
    emit q->selectionChanged();
    //emitCursorPositionChanged();
}

bool SlackTextPrivate::finishChange(bool update)
{
    Q_Q(SlackText);

    Q_UNUSED(update)
    //#if QT_CONFIG(im)
    //    bool inputMethodAttributesChanged = m_textDirty || m_selDirty;
    //#endif
    bool alignmentChanged = false;
    bool textChanged = false;

    //    if (m_textDirty) {
    //        // do validation
    //        bool wasValidInput = m_validInput;
    //        bool wasAcceptable = m_acceptableInput;
    //        m_validInput = true;
    //        m_acceptableInput = true;
    //#if QT_CONFIG(validator)
    //        if (m_validator) {
    //            QString textCopy = m_text;
    //            if (m_maskData)
    //                textCopy = maskString(0, m_text, true);
    //            int cursorCopy = m_cursor;
    //            QValidator::State state = m_validator->validate(textCopy, cursorCopy);
    //            if (m_maskData)
    //                textCopy = m_text;
    //            m_validInput = state != QValidator::Invalid;
    //            m_acceptableInput = state == QValidator::Acceptable;
    //            if (m_validInput && !m_maskData) {
    //                if (m_text != textCopy) {
    //                    internalSetText(textCopy, cursorCopy);
    //                    return true;
    //                }
    //                m_cursor = cursorCopy;
    //            }
    //        }
    //#endif
    //        if (m_maskData)
    //            checkIsValid();

    //        if (validateFromState >= 0 && wasValidInput && !m_validInput) {
    //            if (m_transactions.count())
    //                return false;
    //            internalUndo(validateFromState);
    //            m_history.resize(m_undoState);
    //            m_validInput = true;
    //            m_acceptableInput = wasAcceptable;
    //            m_textDirty = false;
    //        }

    //        if (m_textDirty) {
    //            textChanged = true;
    //            m_textDirty = false;
    //#if QT_CONFIG(im)
    //            m_preeditDirty = false;
    //#endif
    //            alignmentChanged = determineHorizontalAlignment();
    //            if (edited)
    //                emit q->textEdited();
    //            emit q->textChanged();
    //        }

    //        updateDisplayText(alignmentChanged);

    //        if (m_acceptableInput != wasAcceptable)
    //            emit q->acceptableInputChanged();
    //    }
    //#if QT_CONFIG(im)
    //    if (m_preeditDirty) {
    //        m_preeditDirty = false;
    //        if (determineHorizontalAlignment()) {
    //            alignmentChanged = true;
    //            updateLayout();
    //        }
    //    }
    //#endif

    if (m_selDirty) {
        m_selDirty = false;
        emit q->selectionChanged();
    }

    //#if QT_CONFIG(im)
    //    inputMethodAttributesChanged |= (m_cursor != m_lastCursorPos);
    //    if (inputMethodAttributesChanged)
    //        q->updateInputMethod();
    //#endif
    //    emitUndoRedoChanged();

    //    if (!emitCursorPositionChanged() && (alignmentChanged || textChanged))
    //        q->updateCursorRectangle();

    return true;
}

void SlackTextPrivate::updateLayout()
{
    Q_Q(SlackText);

    if (!q->isComponentComplete())
        return;

    m_lp->updateLayout();
}

void SlackTextPrivate::processKeyEvent(QKeyEvent* event)
{
    Q_Q(SlackText);

    if (event == QKeySequence::Copy) {
        copy();
        event->accept();
    } else {
        event->ignore();
    }
}
