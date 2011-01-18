#include "codemap.h"

Node* CodeMap::addPath( QString path ) {
    Node* current = this->root ;
    for ( int i = 0; i < path.length(); i++ ) {
        QChar code = path[i] ;
        Node* next = current->addChild( code ) ;
        current = next ;
    }
    return current ;
}

void CodeMap::insertRecord( QString path, QString pinyin, QString hanzi, qreal freq ) {
    Node* node = this->addPath( path ) ;
    RecordList* list = node->addRecord( pinyin ) ;
    list->insertRecord( hanzi, freq ) ;
}

Node* CodeMap::seek( QString path ) {
    Node* current = this->root ;
    for ( int i = 0; i < path.length(); i++ ) {
        QChar code = path[i] ;
        Node* next = current->findChild( code ) ;
        if ( next )
            current = next ;
        else 
            return NULL ;
    }
    return current ;
}

