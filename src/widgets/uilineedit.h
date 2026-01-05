#ifndef UILINEEDIT_H
#define UILINEEDIT_H

#include "uiabstractbutton.h"
#include "uiconhandler.h"
#include <vector>


class DECL_VISUALUI UILineEdit : public UIAbstractButton, public UIconHandler
{
public:
    explicit UILineEdit(UIWidget *parent = nullptr, const tstring &text = {});
    ~UILineEdit();

    virtual void setText(const tstring &text) noexcept override;
    void setPlaceholderText(const tstring &text) noexcept;
    void setEditable(bool editable);

protected:
#ifdef _WIN32
    virtual bool event(UINT, WPARAM, LPARAM, LRESULT*) override;
#else
    virtual bool event(uint ev_type, void *param) override;
#endif
    virtual void onPaint(const RECT &rc) override;

private:
    const tstring& displayedText() const;
    void calculateCaretPositions();
    void updateViewportAndCaret();
    void updateCaretOnMouseClick(int x, int y);
    void updateCaretPosition();
    void defaultCaretRect(Rect &rc);

    tstring m_sourceText,
            m_placeholderText;
#ifdef _WIN32
    bool m_caretCreated;
#else
    GtkWidget *m_caret;
#endif
    Rect m_viewportRc;
    bool m_editable;
    size_t m_pos;
    int  m_caretPosX,
         m_caretPosY,
         m_textOffsetX;
    std::vector<Point> m_caretPositions;
};

#endif // UILINEEDIT_H
