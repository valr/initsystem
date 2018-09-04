
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/uio.h>

typedef char sd_journal;

/* sd-journal */

extern int __attribute__ ((format (printf, 2, 3))) sd_journal_print(int priority, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vsyslog(priority, format, ap);
    va_end(ap);
    return 0;
}

extern int __attribute__ ((format (printf, 2, 0))) sd_journal_printv(int priority, const char *format, va_list ap) {
    vsyslog(priority, format, ap);
    return 0;
}

extern int __attribute__ ((format (printf, 1, 0))) __attribute__((sentinel)) sd_journal_send(const char *format, ...) {
    return 0;
}

extern int sd_journal_sendv(const struct iovec *iov, int n) {
    return 0;
}

extern int sd_journal_stream_fd(const char *identifier, int priority, int level_prefix) {
    return -1;
}

extern int sd_journal_open(sd_journal **ret, int flags) {
    return 0;
}

extern void sd_journal_close(sd_journal *j) {
    return;
}

extern int sd_journal_next(sd_journal *j) {
    return 0;
}

extern int sd_journal_previous_skip(sd_journal *j, uint64_t skip) {
    return 0;
}

extern int sd_journal_get_realtime_usec(sd_journal *j, uint64_t *ret) {
    *ret = 0;
    return 0;
}

extern int sd_journal_get_data(sd_journal *j, const char *field, const void **data, size_t *l) {
    return -ENOENT;
}

extern int sd_journal_add_match(sd_journal *j, const void *data, size_t size) {
    return 0;
}

extern void sd_journal_flush_matches(sd_journal *j) {
    return;
}

extern int sd_journal_seek_tail(sd_journal *j) {
    return 0;
}

extern int sd_journal_open_directory(sd_journal **ret, const char *path, int flags) {
    return 0;
}

/* sd-daemon */

extern int sd_booted(void) {
    return 0;
}

extern int sd_is_socket(int fd, int family, int type, int listening) {
    return 0;
}

extern int sd_listen_fds(int unset_environment) {
    return 0;
}

/* sd-login */

extern int sd_pid_get_owner_uid(pid_t pid, uid_t *uid) {
    return -ENODATA;
}

extern int sd_pid_get_session(pid_t pid, char **session) {
    return -ENODATA;
}

extern int sd_session_get_seat(const char *session, char **seat) {
    return -ENODATA;
}

extern int sd_uid_get_seats(uid_t uid, int require_active, char ***seats) {
    return -ENODATA;
}

extern int sd_pid_get_unit(pid_t pid, char **unit) {
    return -ENODATA;
}

extern int sd_pid_get_user_unit(pid_t pid, char **unit) {
    return -ENODATA;
}

extern int sd_pid_get_machine_name(pid_t pid, char **name) {
    return -ENODATA;
}

extern int sd_pid_get_slice(pid_t pid, char **slice) {
    return -ENODATA;
}
