#ifndef INPUTMETHOD_H
#define INPUTMETHOD_H

//#include "toolbar/data.h"

#include <mabstractinputmethod.h>

#include <QRect>

namespace inputmethod {

class InputMethodPrivate ;

class InputMethod : public MAbstractInputMethod {
    Q_OBJECT
    Q_PROPERTY( int screenWidth READ screenWidth NOTIFY screenWidthChanged )
    Q_PROPERTY( int screenHeight READ screenHeight NOTIFY screenHeightChanged )
    Q_PROPERTY( int appOrientation READ appOrientation NOTIFY appOrientationChanged )
    Q_PROPERTY( QRect cursorRect READ cursorRect NOTIFY cursorRectChanged )
    Q_PROPERTY( bool useHardwareKeyboard READ useHardwareKeyboard NOTIFY useHardwareKeyboardChanged )
    Q_PROPERTY( QString debugString READ debugString NOTIFY debugStringChanged )

signals :
    void screenWidthChanged( int width ) ;
    void screenHeightChanged( int height ) ;
    void appOrientationChanged( int angle ) ;
    void cursorRectChanged( const QRect& cursorRect ) ;
    void debugStringChanged( const QString& debugString ) ;
    void useHardwareKeyboardChanged( bool useHardwareKeyboard ) ;

public :
    explicit InputMethod( MAbstractInputMethodHost *host, QWidget *mainWindow ) ;
    virtual ~InputMethod() ;

    virtual void show() ;
    virtual void hide() ;
    virtual void setPreedit( const QString &preeditString, int cursorPos ) ;
    virtual void update() ;
    virtual void reset() ;
    virtual void handleMouseClickOnPreedit( const QPoint &pos, const QRect &preeditRect ) ;
    virtual void handleFocusChange( bool focusIn ) ;
    virtual void handleVisualizationPriorityChange( bool priority ) ;
    virtual void handleAppOrientationAboutToChange( int angle ) ;
    virtual void handleAppOrientationChanged( int angle ) ;
    virtual void setToolbar( QSharedPointer<const MToolbarData> toolbar ) ;
    virtual void processKeyEvent( QEvent::Type keyType, Qt::Key keyCode, Qt::KeyboardModifiers modifiers, const QString& text, bool autoRepeat, int count, quint32 nativeScanCode, quint32 nativeModifiers, unsigned long time ) ;
    virtual void setState( const QSet<MInputMethod::HandlerState> &state ) ;
    virtual void handleClientChange() ;
    virtual void switchContext( MInputMethod::SwitchDirection direction, bool enableAnimation ) ;
    virtual QList<MInputMethodSubView> subViews( MInputMethod::HandlerState state = MInputMethod::OnScreen ) const ;
    virtual void setActiveSubView( const QString &subViewId, MInputMethod::HandlerState state = MInputMethod::OnScreen ) ;
    virtual QString activeSubView( MInputMethod::HandlerState state = MInputMethod::OnScreen ) const ;
    virtual void showLanguageNotification() ;
    virtual void setKeyOverrides( const QMap<QString, QSharedPointer<MKeyOverride> > &overrides ) ;
    virtual bool imExtensionEvent( MImExtensionEvent *event ) ;

    int screenWidth() const ;
    int screenHeight() const ;
    int appOrientation() const ;
    const QRect& cursorRect() const ;
    bool useHardwareKeyboard() const ;
    const QString& debugString() const ;

public slots:
    void remapKey( int src, int dest ) ;
    void unrampKey( int src ) ;
    void remapSymbol( const QString& src, const QString& dest ) ;
    void unramapSymbol( const QString& src ) ;
    void remapStickySymbol( const QString& mask, const QString& src, const QString& dest ) ;
    void unremapStickySymbol( const QString& mask, const QString& src ) ;

    void setScreenRegion( const QRect &area ) ;
    void setInputMethodArea( const QRect &area ) ;
    void sendPreedit( const QString& black, const QString& red ) ;
    void sendCommit( const QString& text ) ;
    void processKeyEvent( int keyType, int keyCode, int modifiers, const QString& text, bool autoRepeat, int count ) ;

private :
    Q_DISABLE_COPY( InputMethod ) ;
    Q_DECLARE_PRIVATE( InputMethod ) ;
    InputMethodPrivate* const d_ptr ;
} ;

}
#endif
