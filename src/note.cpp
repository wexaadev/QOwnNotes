#include "note.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMessageBox>
#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QSqlError>


Note::Note()
{
    this->id = 0;
}

int Note::getId()
{
    return this->id;
}

QString Note::getName()
{
    return this->name;
}

QString Note::getFileName()
{
    return this->fileName;
}

QString Note::getNoteText()
{
    return this->noteText;
}

bool Note::createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
    {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                      "This example needs SQLite support. Please read "
                      "the Qt SQL driver documentation for information how "
                      "to build it.\n\n"
                      "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    QSqlQuery query;
    query.exec("create table note (id integer primary key, "
            "name varchar(255), file_name varchar(255), note_text text,"
            "has_dirty_data integer default 0,"
            "file_last_modified datetime,"
            "file_created datetime,"
            "created datetime default current_timestamp,"
            "modified datetime default current_timestamp)");

    return true;
}

bool Note::addNote( QString name, QString fileName, QString text )
{
    QSqlQuery query;
    query.prepare( "INSERT INTO note ( name, file_name, note_text ) VALUES ( :name, :file_name, :note_text )" );
    query.bindValue( ":name", name );
    query.bindValue( ":file_name", fileName );
    query.bindValue( ":note_text", text );
    return query.exec();
}

Note Note::fetch( int id )
{
    QSqlQuery query;
    Note note;

    query.prepare( "SELECT * FROM note WHERE id = :id" );
    query.bindValue( ":id", id );

    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        query.first();
        note = noteFromQuery( query );
    }

    return note;
}

Note Note::fetchByFileName( QString fileName )
{
    QSqlQuery query;
    Note note;

    query.prepare( "SELECT * FROM note WHERE file_name = :file_name" );
    query.bindValue( ":file_name", fileName );

    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        query.first();
        note = noteFromQuery( query );
    }

    return note;
}

Note Note::noteFromQuery( QSqlQuery query )
{
    int id = query.value("id").toInt();
    QString name = query.value("name").toString();
    QString fileName = query.value("file_name").toString();
    QString noteText = query.value("note_text").toString();
    bool hasDirtyData = query.value("has_dirty_data").toInt() == 1;
    QDateTime fileCreated = query.value("file_created").toDateTime();
    QDateTime fileLastModified = query.value("file_last_modified").toDateTime();
    QDateTime created = query.value("created").toDateTime();
    QDateTime modified = query.value("modified").toDateTime();

    Note note;
    note.id = id;
    note.name = name;
    note.fileName = fileName;
    note.noteText = noteText;
    note.hasDirtyData = hasDirtyData;
    note.fileCreated = fileCreated;
    note.fileLastModified = fileLastModified;
    note.created = created;
    note.modified = modified;

    return note;
}

QList<Note> Note::fetchAll()
{
    QSqlQuery query;
    QList<Note> noteList;

    query.prepare( "SELECT * FROM note ORDER BY file_last_modified DESC" );
    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            Note note = noteFromQuery( query );
            noteList.append( note );
        }
    }

    return noteList;
}

QList<Note> Note::search( QString text )
{
    QSqlQuery query;
    QList<Note> noteList;

    query.prepare( "SELECT * FROM note WHERE note_text LIKE :text ORDER BY file_last_modified DESC" );
    query.bindValue( ":text", "%" + text + "%"  );

    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            Note note = noteFromQuery( query );
            noteList.append( note );
        }
    }

    return noteList;
}

QList<QString> Note::searchAsNameList( QString text )
{
    QSqlQuery query;
    QList<QString> nameList;

    query.prepare( "SELECT name FROM note WHERE note_text LIKE :text ORDER BY file_last_modified DESC" );
    query.bindValue( ":text", "%" + text + "%"  );

    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            nameList.append( query.value("name").toString() );
        }
    }

    return nameList;
}

QStringList Note::fetchNoteNames()
{
    QSqlQuery query;
    QStringList list;

    query.prepare( "SELECT name FROM note ORDER BY file_last_modified DESC" );
    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            list.append( query.value("name").toString() );
        }
    }

    return list;
}

QStringList Note::fetchNoteFileNames()
{
    QSqlQuery query;
    QStringList list;

    query.prepare( "SELECT file_name FROM note ORDER BY file_last_modified DESC" );
    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            list.append( query.value("file_name").toString() );
        }
    }

    return list;
}

bool Note::storeNewText( QString text ) {
    this->noteText = text;
    this->hasDirtyData = true;

    return this->store();
}

//
// inserts or updates a note object in the database
//
bool Note::store() {
    QSqlQuery query;

    if ( this->id > 0 )
    {
        query.prepare( "UPDATE note SET "
                       "name = :name, file_name = :file_name, note_text = :note_text, has_dirty_data = :has_dirty_data, "
                       "file_last_modified = :file_last_modified, file_created = :file_created, modified = :modified "
                       "WHERE id = :id" );
        query.bindValue( ":id", this->id );
    }
    else
    {
        query.prepare( "INSERT INTO note"
                       "( name, file_name, note_text, has_dirty_data, file_last_modified, file_created, modified ) "
                       "VALUES ( :name, :file_name, :note_text, :has_dirty_data, :file_last_modified, :file_created, :modified )");
    }

    QDateTime modified = QDateTime::currentDateTime();

    query.bindValue( ":name", this->name );
    query.bindValue( ":file_name", this->fileName );
    query.bindValue( ":note_text", this->noteText );
    query.bindValue( ":has_dirty_data", this->hasDirtyData ? 1 : 0 );
    query.bindValue( ":file_created", this->fileCreated );
    query.bindValue( ":file_last_modified", this->fileLastModified );
    query.bindValue( ":modified", modified );

    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
        return false;
    }
    else
    {
        this->id = query.lastInsertId().toInt();
        this->modified = modified;
        return true;
    }
}

bool Note::storeNoteTextFileToDisk() {

    QFile file( fullNoteFilePath( this->fileName ) );
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
        qDebug() << file.errorString();
        return false;
    }

    QTextStream out( &file );
    out << this->noteText;
    file.flush();
    file.close();

    this->hasDirtyData = false;
    return this->store();
}

//QString Note::generateFilename() {

//}

bool Note::updateNoteTextFromDisk() {
    QFile file( fullNoteFilePath( this->fileName ) );

    if ( !file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << file.errorString();
        return false;
    }

    QTextStream in( &file );
    this->noteText = in.readAll();
    file.close();

    return true;
}

QString Note::fullNoteFilePath( QString fileName )
{
    QSettings settings( "PBE", "QOwnNotes" );
    QString notesPath = settings.value( "General/notesPath" ).toString();

    return notesPath + QDir::separator() + fileName;
}

bool Note::storeDirtyNotesToDisk() {
    QSqlQuery query;
    Note note;

    query.prepare( "SELECT * FROM note WHERE has_dirty_data = 1" );
    if( !query.exec() )
    {
        qDebug() << __func__ << ": " << query.lastError();
        return false;
    }
    else
    {
        for( int r=0; query.next(); r++ )
        {
            note = noteFromQuery( query );
            note.storeNoteTextFileToDisk();
//            qDebug() << note.getName();
        }

        return true;
    }
}

bool Note::createFromFile( QFile &file ) {
    if ( file.open( QIODevice::ReadOnly ) )
    {
        QTextStream in( &file );

        // qDebug() << file.size() << in.readAll();
        QString noteText = in.readAll();
        file.close();

        QFileInfo fileInfo;
        fileInfo.setFile( file );

        // create a nicer name by removing ".txt"
        // TODO: make sure name is ownNote conform
        QString name = fileInfo.fileName();
        name.chop( 4 );

        this->name = name;
        this->fileName = fileInfo.fileName();
        this->noteText = noteText;
        this->fileCreated = fileInfo.created();
        this->fileLastModified = fileInfo.lastModified();
        this->store();
    }

}

QDebug operator<<(QDebug dbg, const Note &note)
{
    dbg.nospace() << note.name << "<" << note.id << ">";
    return dbg.space();
}