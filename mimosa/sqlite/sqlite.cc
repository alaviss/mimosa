#include <utility>

#include "../log/log.hh"
#include "../log/origin.hh"
#include "sqlite.hh"

namespace mimosa
{
  namespace sqlite
  {
    log::Origin * log_sqlite = new log::Origin("sqlite", log::kWarning);

    /////////////
    // Db
    /////////////

    Db::Db(sqlite3 * db)
      : db_(db)
    {
    }

    Db::~Db()
    {
      if (db_)
      {
        sqlite3_close(db_);
        db_ = nullptr;
      }
    }

    int
    Db::open(const char * filename,
             int          flags,
             const char * vfs)
    {
      assert(!db_);
      int err = sqlite3_open_v2(filename, &db_, flags, vfs);

      if (err != SQLITE_OK)
      {
        log_sqlite->error("failed to open database: %s: %s",
                          filename, sqlite3_errmsg(db_));
        sqlite3_close(db_);
        db_ = nullptr;
      }
      return err;
    }

    Stmt
    Db::prepare(const char *str, int len)
    {
      Stmt stmt;
      stmt.prepare(db_, str, len);
      return stmt;
    }

    /////////////
    // Stmt
    /////////////

    Stmt::Stmt(sqlite3_stmt * stmt)
      : stmt_(stmt)
    {
    }

    Stmt::Stmt(Stmt && stmt)
      : NonCopyable(std::move(stmt)),
        stmt_(stmt.stmt_)
    {
      stmt.stmt_ = nullptr;
    }

    Stmt &
    Stmt::operator=(Stmt && stmt)
    {
      std::swap(stmt_, stmt.stmt_);
      return *this;
    }

    Stmt::~Stmt()
    {
      if (stmt_)
      {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
      }
    }

    Stmt &
    Stmt::prepare(sqlite3 *    db,
                  const char * sql,
                  int          sql_size)
    {
      assert(!stmt_);
      int err = sqlite3_prepare_v2(db, sql, sql_size, &stmt_, nullptr);
      if (err != SQLITE_OK)
      {
        log_sqlite->error("failed to prepare statement (error: %d): %s",
                          err, sql);
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
        throw nullptr;
      }
      assert(stmt_);
      return *this;
    }

    int
    Stmt::step()
    {
      assert(stmt_);
      return sqlite3_step(stmt_);
    }

    void
    Stmt::reset()
    {
      assert(stmt_);
      sqlite3_reset(stmt_);
    }
  }
}
