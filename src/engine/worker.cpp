#include "worker.h"
#include "lookup/lookup.h"
#include "lookup/t9.h"

#include <QStringList>
#include <QSet>

//#include <QDebug>

namespace engine {

inline void append_selected( SelectedPair* pair, const QString* key, const QString* preedit, const QString* word, qreal freq ) {
    if ( !pair->first.first.isEmpty() ) {
        pair->first.first.append( QChar('\'') ) ;
        pair->first.second.append( QChar('\'') ) ;
    }
    pair->first.first.append( *key ) ;
    pair->first.second.append( *preedit ) ;
    pair->second.first.append( *word ) ;
    int count = key->count( QChar('\'') ) ;
    for ( int i = 0 ; i < count ; i ++ ) ;
        pair->second.second.append( -1 ) ;
    pair->second.second.append( freq ) ;
}

inline void clear_selected( SelectedPair* pair ) {
    pair->first.first.clear() ;
    pair->first.second.clear() ;
    pair->second.first.clear() ;
    pair->second.second.clear() ;
}

inline void pop_selected( SelectedPair* pair, QString* code ) {
    if ( pair->second.first.length() > 1 ) {
        int index ;
        int length ;
        
        //qDebug() << *pair ;
        index = pair->first.first.lastIndexOf( QChar('\'') ) ;
        length = pair->first.first.length() ;
        pair->first.first.chop( length - index ) ;

        index = pair->first.second.lastIndexOf( QChar('\'') ) ;
        length = pair->first.second.length() ;
        *code = pair->first.second.right( length - index - 1 ) ;
        pair->first.second.chop( length - index ) ;

        pair->second.first.chop( 1 ) ;
        pair->second.second.removeLast() ;
    }
    else {
        *code = pair->first.second ;
        clear_selected( pair ) ;
    }
}

Worker::Worker() : 
    lookup( new lookup::Lookup() ),
    t9lookup( new t9::T9Lookup( &(this->lookup->dict) ) ),
    selected(),
    selectedWord( &(this->selected.second.first) ) {
    this->pageStartIndex = 0 ;
    this->candidate = NULL ;
    this->keyboardLayout = UnknownKeyboardLayout ;
    //this->logFile = NULL ;
    //this->textStream = NULL ;

    //this->flushTimer.setInterval( 1200000 ) ;
    //QObject::connect( &(this->flushTimer), SIGNAL(timeout()), this, SLOT(flushLog() ) ) ;
}

Worker::~Worker() {
    //if ( this->logFile ) {
        //delete this->textStream ;
        //this->logFile->close() ;
        //delete this->logFile ;
    //}
}

//void Worker::startLog( const QString& path ) {
    //this->logFile = new QFile( path ) ;
    //if ( this->logFile->exists() ) {
        //this->logFile->open( QIODevice::WriteOnly | QIODevice::Append ) ;
        //this->textStream = new QTextStream( this->logFile ) ;
        //this->textStream->setCodec( "utf-8" ) ;
        //this->textStream->setRealNumberNotation( QTextStream::SmartNotation ) ;
        //this->textStream->setRealNumberPrecision( 16 ) ;
        //this->flushTimer.start() ;
    //}
    //else {
        //delete this->logFile ;
        //this->logFile = NULL ;
    //}
//}

//void Worker::stopLog() {
    //if ( this->logFile ) {
        //delete this->textStream ;
        //this->logFile->close() ;
        //delete this->logFile ;
        //this->logFile = NULL ;
        //this->textStream = NULL ;
        //this->flushTimer.stop() ;
    //}
//}

//void Worker::flushLog() {
    //if ( this->logFile ) 
        //this->logFile->flush() ;
//}

void Worker::load( const QString& path ) {
    QFile file( path ) ;
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        QSet<QString> newKeySet ;
        QTextStream in( &file ) ;
        in.setCodec( "utf-8" ) ;
        while( !in.atEnd() ) {
            QString line = in.readLine() ;
            QStringList list = line.split( " " ) ;
            if ( list.length() == 3 ) {
                QString key = list.at(0) ;
                QString word = list.at(1) ;
                qreal freq = list.at(2).toDouble() ;
                //qDebug() << key << word << freq ;
                if ( !this->lookup->dict.hash.contains( key ) )
                    newKeySet.insert( key ) ;
                this->lookup->dict.insert( key, word, freq ) ;
            }
        }
        foreach( const QString& key, newKeySet ) {
            if ( key.count( '\'' ) <= 0 )
                split::add_key( &(this->lookup->spliter.keySet), key ) ;
            fit::add_key( &(this->lookup->keyMap), key ) ;
            this->t9lookup->tree.addKey( key ) ;
        }
    }
}

bool Worker::prevPage( int pageLength ) {
    bool flag = false ;
    if ( this->pageStartIndex >= pageLength ) {
        this->pageStartIndex -= pageLength ;
        if ( this->pageStartIndex < 0 )
            this->pageStartIndex = 0 ;
        flag = true ;
    }
    return flag ;
}

bool Worker::nextPage( int pageLength ) {
    bool flag = false ;
    if ( this->getCodeLength() > 0 ) {
        int pageStartIndex = this->pageStartIndex + pageLength ;
        const lookup::Candidate* candidate ;
        if ( this->keyboardLayout == FullKeyboardLayout )
            candidate = this->lookup->getCandidate( pageStartIndex ) ;
        else if ( this->keyboardLayout == T9KeyboardLayout )
            candidate = this->t9lookup->getCandidate( pageStartIndex ) ;
        else
            candidate = NULL ;
        if ( candidate ) {
            this->pageStartIndex = pageStartIndex ;
            flag = true ;
        }
    }
    return flag ;
}

int Worker::getCodeLength() const {
    if ( this->keyboardLayout == FullKeyboardLayout )
        return this->lookup->spliter.code.length() ;
    else if ( this->keyboardLayout == T9KeyboardLayout )
        return this->t9lookup->code.length() ;
    else
        return 0 ;
}

int Worker::getPreeditCodeLength() const {
    if ( this->candidate ) {
        const QString* preedit = lookup::get_preedit( this->candidate ) ;
        return preedit->length() - preedit->count( '\'' ) ;
    }
    return 0 ;
}

int Worker::getInvalidCodeLength() const { return this->getCodeLength() - this->getPreeditCodeLength() ; }

int Worker::getSelectedWordLength() const { return this->selectedWord->length() ; }

bool Worker::updateCandidate( int index ) {
    if ( this->keyboardLayout == FullKeyboardLayout ) {
        if ( this->lookup->spliter.code.isEmpty() )
            this->candidate = NULL ;
        else 
            this->candidate = this->lookup->getCandidate( pageStartIndex + index ) ;
    }
    else if ( this->keyboardLayout == T9KeyboardLayout ) {
        if ( this->t9lookup->code.isEmpty() )
            this->candidate = NULL ;
        else 
            this->candidate = this->t9lookup->getCandidate( pageStartIndex + index ) ;
    }
    return this->candidate ;
    //if ( this->candidate ) 
        //qDebug() << *(lookup::get_key( this->candidate )) << *(lookup::get_preedit( this->candidate )) << *(lookup::get_word( this->candidate )) << lookup::get_freq( this->candidate ) ;
}


QString Worker::getCode() const {
    if ( this->keyboardLayout == FullKeyboardLayout ) 
        return this->lookup->spliter.code ;
    else if ( this->keyboardLayout == T9KeyboardLayout ) 
        return this->t9lookup->code ;
    else
        return "" ;
}

QString Worker::getWord() const {
    if ( this->candidate ) 
        return *(lookup::get_word( this->candidate )) ;
    else 
        return "" ;
}

QString Worker::getPreeditCode() const {
    if ( this->candidate ) 
        return *(lookup::get_preedit( this->candidate )) ;
    else
        return "" ;
}

QString Worker::getInvalidCode() const {
    if ( this->keyboardLayout == FullKeyboardLayout ) {
        if ( this->lookup->spliter.code.isEmpty() )
            return "" ;
        else
            return this->lookup->spliter.code.right( this->getInvalidCodeLength() ) ;
    }
    else if ( this->keyboardLayout == T9KeyboardLayout ) {
        if ( this->t9lookup->code.isEmpty() )
            return "" ;
        else
            return this->t9lookup->code.right( this->getInvalidCodeLength() ) ;
    }
    else
        return "" ;
}

QString Worker::getSelectedWord() const { return *(this->selectedWord) ; }

//bool Worker::checkCommit() {
    //bool flag = false ;
    //if ( !this->selectedWord->isEmpty() && this->getCodeLength() <= 0 ) {
        //emit this->sendCommit( *(this->selectedWord) ) ;
        //emit this->preeditEnd() ;
        //this->commit() ;
        //flag = true ;
    //}
    //return flag ;
//}

bool Worker::select( int index ) {
    bool flag = false ;
    if ( this->getCodeLength() > 0 ) {
        if ( this->keyboardLayout == FullKeyboardLayout ) {
            index = this->pageStartIndex + index ;
            const lookup::Candidate* candidate = this->lookup->getCandidate( index ) ;

            if ( candidate ) {
                qreal freq = -0x1000 ;
                if ( this->selectedWord->isEmpty() ) {
                    int halfIndex = ( candidate->second + index ) / 2 ;
                    if ( halfIndex < 2 ) 
                        halfIndex = 0 ;
                    const lookup::Candidate* candidate = this->lookup->getCandidate( halfIndex ) ;
                    freq = lookup::get_freq( candidate ) ;
                    if ( freq <= 0.1 ) 
                        freq = 1.1 ;
                    else 
                        freq += 1 ;
                }
                
                const QString* key = lookup::get_key( candidate ) ;
                const QString* preedit = lookup::get_preedit( candidate ) ;
                const QString* word = lookup::get_word( candidate ) ;
                append_selected( &(this->selected), key, preedit, word, -0x1000 ) ;
                this->selected.second.second.last() = freq ;

                this->candidate = candidate ;
                if ( this->getInvalidCodeLength() > 0 ) {
                    QString code( this->getInvalidCode() ) ;
                    //qDebug() << "r" << code ; 
                    this->lookup->clearCode() ;
                    this->lookup->setCode( code ) ;
                    this->pageStartIndex = 0 ;
                }
                else {
                    this->lookup->clearCode() ;
                    this->pageStartIndex = 0 ;
                }

                flag = true ;
            }
            //else ;
        }
        else if ( this->keyboardLayout == T9KeyboardLayout ) {
            index = this->pageStartIndex + index ;
            const lookup::Candidate* candidate = this->t9lookup->getCandidate( index ) ;

            if ( candidate ) {
                qreal freq = -0x1000 ;
                if ( this->selectedWord->isEmpty() ) {
                    int halfIndex = ( candidate->second + index ) / 2 ;
                    if ( halfIndex < 2 ) 
                        halfIndex = 0 ;
                    const lookup::Candidate* candidate = this->t9lookup->getCandidate( halfIndex ) ;
                    freq = lookup::get_freq( candidate ) ;
                    if ( freq <= 0.1 ) 
                        freq = 1.1 ;
                    else 
                        freq += 1 ;
                }
                
                const QString* key = lookup::get_key( candidate ) ;
                const QString* preedit = lookup::get_preedit( candidate ) ;
                const QString* word = lookup::get_word( candidate ) ;
                append_selected( &(this->selected), key, preedit, word, -0x1000 ) ;
                this->selected.second.second.last() = freq ;

                this->candidate = candidate ;
                if ( this->getInvalidCodeLength() > 0 ) {
                    QString code( this->getInvalidCode() ) ;
                    //qDebug() << "r" << code ; 
                    this->t9lookup->clearCode() ;
                    this->t9lookup->setCode( code ) ;
                    this->pageStartIndex = 0 ;
                }
                else {
                    this->t9lookup->clearCode() ;
                    this->pageStartIndex = 0 ;
                }
                flag = true ;
            }
            //else ;
        }
    }
    return flag ;
}
bool Worker::deselect() {
    bool flag = false ;
    if ( !this->selectedWord->isEmpty() ) {
        QString code ;
        pop_selected( &(this->selected), &code ) ;

        if ( this->keyboardLayout == FullKeyboardLayout ) {
            code.append( this->lookup->spliter.code ) ;
            this->lookup->clearCode() ;
            this->lookup->setCode( code ) ;
            this->pageStartIndex = 0 ;
        }
        else if ( this->keyboardLayout == T9KeyboardLayout ) {
            for ( int i = 0 ; i < code.length() ; i++ ) 
                code[i] = this->t9lookup->tree.trans[ code.at(i).toAscii() - 'a' ] ;
            code.append( this->t9lookup->code ) ;
            this->t9lookup->clearCode() ;
            this->t9lookup->setCode( code ) ;
            this->pageStartIndex = 0 ;
        }

        flag = true ;
    }
    return flag ;
}
void Worker::reset() {
    if ( this->keyboardLayout == FullKeyboardLayout ) 
        this->lookup->clearCode() ;
    else if ( this->keyboardLayout == T9KeyboardLayout ) 
        this->t9lookup->clearCode() ;
    clear_selected( &(this->selected) ) ;
    this->pageStartIndex = 0 ;
}

bool Worker::appendCode( QChar code ) {
    bool flag = false ;
    if ( this->keyboardLayout == FullKeyboardLayout ) {
        if ( code >= 'a' && code <= 'z' ) {
            //if ( this->lookup->spliter.code.isEmpty() )
                //emit this->preeditStart() ;
            this->lookup->appendCode( code ) ;
            this->pageStartIndex = 0 ;
            flag = true ;
        }
    }
    else if ( this->keyboardLayout == T9KeyboardLayout ) {
        if ( code >= '2' && code <= '9' ) {
            //if ( this->t9lookup->code.isEmpty() )
                //emit this->preeditStart() ;
            this->t9lookup->appendCode( code ) ;
            this->pageStartIndex = 0 ;
            flag = true ;
        }
    }
    return flag ;
}

//bool Worker::appendCode( const QString& code ) {
    //return this->appendCode( code.at(0) ) ;
//}

bool Worker::popCode() {
    bool flag = false ;
    if ( this->keyboardLayout == FullKeyboardLayout ) {
        if ( !this->lookup->spliter.code.isEmpty() ) {
            this->lookup->popCode() ;
            this->pageStartIndex = 0 ;
            flag = true ;
        }
    }
    else if ( this->keyboardLayout == T9KeyboardLayout ) {
        if ( !this->t9lookup->code.isEmpty() ) {
            this->t9lookup->popCode() ;
            this->pageStartIndex = 0 ;
            flag = true ;
        }
    }
    return flag ;
}

void Worker::commit() {
    if ( this->selectedWord->length() < 6 ) {
        const QString* key = &(this->selected.first.first) ;
        qreal freq = this->selected.second.second.last() ;
        //freq = this->lookup->dict.update( *key, *(this->selectedWord), freq ) ;
        this->lookup->dict.insert( *key, *(this->selectedWord), freq ) ;
        if ( key->count( QChar('\'') ) <= 0 )
            split::add_key( &(this->lookup->spliter.keySet), *key ) ;
        fit::add_key( &(this->lookup->keyMap), *key ) ;
        this->t9lookup->tree.addKey( *key ) ;
        //if ( this->logFile ) {
            //(*this->textStream) << *key << QChar( ' ' ) << *(this->selectedWord) << QChar( ' ' ) << freq << QChar( '\n' ) ;
            //this->flushLog() ;
        //}
    }
    this->reset() ;
}

bool Worker::setKeyboardLayout( Worker::KeyboardLayout layout ) {
    if ( layout != this->keyboardLayout ) {
        this->reset() ;
        this->keyboardLayout = layout ;
        return true ;
    }
    else
        return false ;
}

//bool Worker::setKeyboardLayout( int layout ) {
    //if ( layout == 0 ) 
        //return this->setKeyboardLayout( UnknownKeyboardLayout ) ;
    //else if ( layout == 1 ) 
        //return this->setKeyboardLayout( FullKeyboardLayout ) ;
    //else if ( layout == 2 ) 
        //return this->setKeyboardLayout( T9KeyboardLayout ) ;
    //else 
        //return false ;
//}

}

