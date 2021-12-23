---
content_title: ksigd Troubleshooting
---

## How to solve the error "Failed to lock access to wallet directory; is another `ksigd` running"?

Since `clsig` may auto-launch an instance of `ksigd`, it is possible to end up with multiple instances of `ksigd` running. That can cause unexpected behavior or the error message above.

To fix this issue, you can terminate all running `ksigd` instances and restart `ksigd`. The following command will find and terminate all instances of `ksigd` running on the system:

```sh
pkill ksigd
```
