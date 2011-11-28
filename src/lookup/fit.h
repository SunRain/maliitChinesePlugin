#ifndef FIT_H
#define FIT_H

#include <QHash>
#include <QString>
#include <QList>
#include <QSet>

//#include <QDebug>

namespace fit {

typedef QHash< QString, QSet<QString>  > KeyMap ;
// KeyMap.key : path to full key, ie (pinyin) : sgr -> san'ge'ri
// KeyMap.value : set of full key

inline void add_key( KeyMap* map, const QString& key ) {
    QString path( key.at(0) ) ;
    int i = key.indexOf( QChar('\'') ) ;
    while ( i > 0 ) {
        i++ ;
        path.append( key.at(i) ) ;
        i = key.indexOf( QChar('\''), i ) ;
    }
    (*map)[path].insert( key ) ;
}

inline const QSet<QString>* get_keys( KeyMap* map, const QString& path ) {
    return map->contains( path ) ? &((*map)[path]) : NULL ;
}

inline void check_string( const QStringList* string, const QString& key, bool* flag, int* fit_point ) {
    QStringList list( key.split( QChar('\'') ) ) ;
    for ( int i = 0 ; i < list.length() && *flag ; i++ ) {
        const QString& s = string->at(i) ;
        const QString& k = list.at(i) ;
        if ( s == k )
            ;
        else {
            (*fit_point)-- ;
            if ( s.length() > k.length() )
                *flag = false ;
            else {
                QString head( k ) ;
                head.truncate( s.length() ) ;
                if ( s != head ) 
                    *flag = false ;
            }
        }
    }
}

inline void fit( const QStringList* string, QList<const QString*>* buffer, int* fit_point, KeyMap* key_map ) {
    QString path ;
    for ( int i = 0, l = string->length() ; i < l ; i++ ) {
        path.append( string->at(i).at(0) ) ;
    }
    
    const QSet<QString>* keys = get_keys( key_map, path ) ;
    int highest_point = -0x1000 ;

    if ( keys ) {
        //qDebug() << *string << *keys ;
        foreach( const QString& key, *keys ) {
            *fit_point = 0 ;
            bool flag = true ;
            check_string( string, key, &flag, fit_point ) ;
            //qDebug() << *string << key << flag << fit_point ;
            if ( flag ) {
                if ( *fit_point >= 0 ) {
                    buffer->clear() ;
                    buffer->append( &key ) ;
                    highest_point = 0 ;
                    break ;
                }
                else if ( *fit_point > highest_point ) {
                    buffer->clear() ;
                    buffer->append( &key ) ;
                    highest_point = *fit_point ;
                }
                else if ( *fit_point == highest_point ) 
                    buffer->append( &key ) ;
            }
        }
    }
    *fit_point = highest_point ;
}

}

#endif
