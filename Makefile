TARGET := libringbuf.so
SRCS := readerWarpper.cpp writerWarpper.cpp \
		ringbuf.cpp writerBase.cpp readerBase.cpp
HDRS := $(SRCS:%.cpp=%.h)

.PHONY: all install header clean
all: $(TARGET)

%.d: %.c
	$(CC) -MM $(CFLAGS) $< | sed -re 's,^.*:,$*.o $*.d:,g' > $@
%.d: %.cpp
	$(CXX) -MM $(CFLAGS) $< | sed -re 's,^.*:,$*.o $*.d:,g' > $@
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

DEPENDS := $(SRCS:%.c=%.d)
DEPENDS := $(DEPENDS:%.cpp=%.d)

header:
	$(INSTALL_DIR) $(DES_DIR)/usr/include/ringbuf
	$(foreach file,$(HDRS),\
        $(INSTALL_BIN) $(PKG_BUILD_DIR)/$(file) $(DES_DIR)/usr/include/ringbuf;)
	$(INSTALL_DIR) $(DES_DIR)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(TARGET) $(DES_DIR)/usr/lib/
	$(LN) $(TARGET) $(DES_DIR)/usr/lib/$(TARGET).$(PKG_VERSION)

install:
	$(INSTALL_DIR) $(DES_DIR)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(TARGET) $(DES_DIR)/usr/lib/
	$(LN) $(TARGET) $(DES_DIR)/usr/lib/$(TARGET).$(PKG_VERSION)

clean:
	rm -f *.d *.o libringbuf.so
    
$(TARGET): $(SRCS:%.c=%.o)
	$(CXX) -shared $^ $(LDFLAGS) -o $@

