#if INTERFACE
#include <Udon/Standard.h>

class Buffer {
public:
    Uint32 ReadPtr;
    Uint32 WritePtr;
    Uint32 Size;
    void** Data;
    void (*FreeFunc)(void*);
};
#endif

#include <Udon/Media/Utils/Buffer.h>

#include <Udon/Logging/Log.h>

PUBLIC Buffer::Buffer(Uint32 size, void (*freeFunc)(void*)) {
    this->ReadPtr = 0;
    this->WritePtr = 0;
    this->Size = size;
    this->FreeFunc = freeFunc;

    this->Data = (void**)calloc(this->Size, sizeof(void*));
    if (this->Data == NULL) {
        Log::Print(Log::LOG_ERROR, "Something went horribly wrong. (Ran out of memory at Buffer::Buffer)");
        exit(-1);
    }
}
PUBLIC Buffer::~Buffer() {
    Clear();
    free(this->Data);
}

PUBLIC Uint32 Buffer::GetLength() {
    return this->WritePtr - this->ReadPtr;
}
PUBLIC void   Buffer::Clear() {
    if (FreeFunc == NULL)
        return;

    void* data;
    while ((data = Read()) != NULL) {
        FreeFunc(data);
    }
}
PUBLIC void*  Buffer::Read() {
    if (ReadPtr < WritePtr) {
        void* out = Data[ReadPtr % Size];
        Data[ReadPtr % Size] = NULL;
        ReadPtr++;
        if (ReadPtr >= Size) {
            ReadPtr = ReadPtr % Size;
            WritePtr = WritePtr % Size;
        }
        return out;
    }
    return NULL;
}
PUBLIC void*  Buffer::Peek() {
    if (ReadPtr < WritePtr) {
        return Data[ReadPtr % Size];
    }
    return NULL;
}
PUBLIC void   Buffer::Advance() {
    if (ReadPtr < WritePtr) {
        Data[ReadPtr % Size] = NULL;
        ReadPtr++;
        if (ReadPtr >= Size) {
            ReadPtr = ReadPtr % Size;
            WritePtr = WritePtr % Size;
        }
    }
}
PUBLIC int    Buffer::Write(void* ptr) {
    assert(ptr != NULL);

    if (!IsFull()) {
        Data[WritePtr % Size] = ptr;
        WritePtr++;
        return 0;
    }
    return 1;
}
PUBLIC void   Buffer::ForEachItemInBuffer(void (*callback)(void*, void*), void* userdata) {
    Uint32 read_p = ReadPtr;
    Uint32 write_p = WritePtr;
    while (read_p < write_p) {
        callback(Data[read_p++ % Size], userdata);
        if (read_p >= Size) {
            read_p = read_p % Size;
            write_p = write_p % Size;
        }
    }
}
PUBLIC void   Buffer::WithEachItemInBuffer(std::function<void(void*, void*)> callback, void* userdata) {
    Uint32 read_p = ReadPtr;
    Uint32 write_p = WritePtr;
    while (read_p < write_p) {
        callback(Data[read_p++ % Size], userdata);
        if (read_p >= Size) {
            read_p = read_p % Size;
            write_p = write_p % Size;
        }
    }
}
PUBLIC int    Buffer::IsFull() {
    int len = WritePtr - ReadPtr;
    int k = (len >= Size);
    return k;
}
