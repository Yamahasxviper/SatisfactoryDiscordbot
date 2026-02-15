# Place Linux libwebsockets library here

Add:
- libwebsockets.a (static library)

Build with:
```bash
cmake .. -DLWS_WITH_STATIC=ON -DLWS_WITH_SHARED=OFF
make
cp lib/libwebsockets.a <here>
```
