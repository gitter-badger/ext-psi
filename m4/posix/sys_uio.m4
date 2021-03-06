PSI_CHECK_SYS_UIO() {

	PSI_STRUCT(struct iovec, [
		void *iov_base,
		size_t iov_len]
	)

	PSI_DECL(ssize_t readv, [(int fd, struct iovec *iov, int iovcnt)])
	PSI_DECL(ssize_t writev, [(int fd, struct iovec *iov, int iovcnt)])
}
