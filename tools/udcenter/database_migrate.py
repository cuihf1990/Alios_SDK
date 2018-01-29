import sys
import sqlite3 as sql

class Migrate():
    def __init__(self):
        self.dbase = sql.connect('controller.db', check_same_thread = False)

    def database_excute_sqlcmd(self, sqlcmd):
        with self.dbase:
            cur = self.dbase.cursor()
            try:
                cur.execute(sqlcmd)
            except:
                #traceback.print_exc()
                ret = None
            else:
                ret = cur.fetchall()
        return ret

    def database_migrate(self):
        sqlcmd = 'SELECT email FROM Users'
        ret = self.database_excute_sqlcmd(sqlcmd)
        if ret == None:
            return False
        sqlcmd = 'ALTER TABLE Users RENAME TO Users_old'
        self.database_excute_sqlcmd(sqlcmd)
        sqlcmd = 'CREATE TABLE Users(uuid TEXT, name TEXT, info TEXT)'
        self.database_excute_sqlcmd(sqlcmd)
        sqlcmd = 'INSERT INTO Users(uuid, name, info) SELECT uuid, name, email FROM Users_old'
        self.database_excute_sqlcmd(sqlcmd)
        sqlcmd = 'DROP TABLE Users_old'
        self.database_excute_sqlcmd(sqlcmd)
        return True

if __name__ == '__main__':
    mg = Migrate()
    ret = mg.database_migrate()
    if ret:
        print "succeed"
        ret = 0
    else:
        print "failed"
        ret = 1
    sys.exit(ret)
