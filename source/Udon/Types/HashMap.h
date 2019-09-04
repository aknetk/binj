#ifndef HASHMAP_H
#define HASHMAP_H

#include <Udon/Standard.h>
#include <Udon/Hashing/FNV1A.h>
#include <Udon/Logging/Log.h>

template <typename T> class HashMapElement {
public:
    Uint32 Key = 0x00000000U;
    bool   Used = false;
    T      Data;
};
template <typename T> class HashMap {
public:
    int Count = 0;
    int Capacity = 0;
    int ChainLength = 8;
    HashMapElement<T>* Data = NULL;

    Uint32 (*HashFunction)(const char*) = NULL;

    HashMap<T>(Uint32 (*hashFunc)(const char*), int capacity = 16) {
        HashFunction = hashFunc;
        if (HashFunction == NULL)
            HashFunction = FNV1A::EncryptString;

        Count = 0;
        Capacity = capacity;

        // Data = (HashMapElement<T>*)Memory::TrackedCalloc("HashMap::Data", Capacity, sizeof(HashMapElement<T>));
        Data = (HashMapElement<T>*)calloc(Capacity, sizeof(HashMapElement<T>));
    	if (!Data) {
            Log::Print(Log::LOG_ERROR, "Could not allocate memory for HashMap data!");
            exit(-1);
        }
    }
    ~HashMap<T>() {
        // Memory::Free(Data);
        free(Data);
    }

    Uint32 TranslateIndex(Uint32 index) {
        // Find index that works to put new data in.
        index += (index << 12);
        index ^= (index >> 22);
        index += (index << 4);
        index ^= (index >> 9);
        index += (index << 10);
        index ^= (index >> 2);
        index += (index << 7);
        index ^= (index >> 12);
        index = (index >> 3) * 0x9E3779B1U;
        index = index & (Capacity - 1); // index = index % Capacity;
        return index;
    }
    int    Resize() {
        Capacity <<= 1;
        HashMapElement<T>* oldData = Data;

        // Data = (HashMapElement<T>*)Memory::TrackedCalloc("HashMap::Data", Capacity, sizeof(HashMapElement<T>));
        Data = (HashMapElement<T>*)calloc(Capacity, sizeof(HashMapElement<T>));
        if (!Data) {
            Log::Print(Log::LOG_ERROR, "Could not allocate memory for HashMap data!");
            exit(-1);
        }

        Count = 0;
        Uint32 index;
        for (int i = 0; i < Capacity / 2; i++) {
            if (oldData[i].Used) {
                index = TranslateIndex(oldData[i].Key);

                for (int c = 0; c < ChainLength; c++) {
                    if (!Data[index].Used) {
                        Count++;
            			break;
                    }
            		if (Data[index].Used && Data[index].Key == oldData[i].Key)
            			break;
            		index = (index + 1) & (Capacity - 1); // index = (index + 1) % Capacity;
            	}

                Data[index].Key = oldData[i].Key;
                Data[index].Used = true;
                Data[index].Data = oldData[i].Data;
            }
        }

        // Memory::Free(oldData);
        free(oldData);

        return Capacity;
    }

    void   Put(Uint32 hash, T data) {
        Uint32 index;
        do {
            index = hash;
            index = TranslateIndex(index);

            for (int i = 0; i < ChainLength; i++) {
                if (!Data[index].Used) {
                    Count++;
                    if (Count >= Capacity / 2) {
                        index = 0xFFFFFFFFU;
                        Resize();
                        break;
                    }
        			break;
                }

        		if (Data[index].Used && Data[index].Key == hash)
        			break;

        		index = (index + 1) & (Capacity - 1); // index = (index + 1) % Capacity;
        	}
        }
        while (index == 0xFFFFFFFFU);

        Data[index].Key = hash;
        Data[index].Used = true;
        Data[index].Data = data;
    }
    void   Put(const char* key, T data) {
        Uint32 hash = HashFunction(key);
        Put(hash, data);
    }
    T      Get(Uint32 hash) {
        Uint32 index;

        index = hash;
        index = TranslateIndex(index);

        for (int i = 0; i < ChainLength; i++) {
            if (Data[index].Used && Data[index].Key == hash) {
                return Data[index].Data;
            }

            index = (index + 1) & (Capacity - 1); // index = (index + 1) % Capacity;
        }

        return NULL;
    }
    T      Get(const char* key) {
        Uint32 hash = HashFunction(key);
        return Get(hash);
    }

    bool   Remove(Uint32 hash) {
        Uint32 index;

        index = hash;
        index = TranslateIndex(index);

        for (int i = 0; i < ChainLength; i++) {
            if (Data[index].Used && Data[index].Key == hash) {
                Count--;
                Data[index].Used = false;
                return true;
            }

            index = (index + 1) & (Capacity - 1); // index = (index + 1) % Capacity;
        }
        return false;
    }
    bool   Remove(const char* key) {
        Uint32 hash = HashFunction(key);
        return Remove(hash);
    }

    void   ForAll(void (*forFunc)(Uint32, T)) {
        for (int i = 0; i < Capacity; i++) {
            if (Data[i].Used) {
                forFunc(Data[i].Key, Data[i].Data);
            }
        }
    }
    void   WithAll(std::function<void(Uint32, T)> forFunc) {
        for (int i = 0; i < Capacity; i++) {
            if (Data[i].Used) {
                forFunc(Data[i].Key, Data[i].Data);
            }
        }
    }

    Uint8* GetBytes(bool exportHashes) {
        Uint32 stride = ((exportHashes ? 4 : 0) + sizeof(T));
        // Uint8* bytes = (Uint8*)Memory::TrackedMalloc("HashMap::GetBytes", Count * stride);
        Uint8* bytes = (Uint8*)malloc(Count * stride);
        if (exportHashes) {
            for (int i = 0, index = 0; i < Capacity; i++) {
                if (Data[i].Used) {
                    *(Uint32*)(bytes + index * stride) = Data[i].Key;
                    *(T*)(bytes + index * stride + 4) = Data[i].Data;
                    index++;
                }
            }
        }
        else {
            for (int i = 0, index = 0; i < Capacity; i++) {
                if (Data[i].Used) {
                    *(T*)(bytes + index * stride) = Data[i].Data;
                    index++;
                }
            }
        }
        return bytes;
    }
    void   FromBytes(Uint8* bytes, int count) {
        Uint32 stride = (4 + sizeof(T));
        for (int i = 0; i < count; i++) {
            Put(*(Uint32*)(bytes + i * stride),
                *(T*)(bytes + i * stride + 4));
        }
    }
};

// template <typename T>
// void   HashMap<T>::Put(Uint32 hash, T data) {
//     Uint32 index;
//     do {
//         index = hash;
//         index = TranslateIndex(index);
//
//         for (int i = 0; i < ChainLength; i++) {
//             if (!Data[index].Used) {
//                 Count++;
//                 if (Count >= Capacity / 2) {
//                     index = 0xFFFFFFFFU;
//                     Resize();
//                     continue;
//                 }
//     			break;
//             }
//
//     		if (Data[index].Used && Data[index].Key == hash)
//     			break;
//
//     		index = (index + 1) & (Capacity - 1); // index = (index + 1) % Capacity;
//     	}
//     }
//     while (index == 0xFFFFFFFFU);
//
//     Data[index].Key = hash;
//     Data[index].Used = true;
//     Data[index].Data = data;
// }

#endif
