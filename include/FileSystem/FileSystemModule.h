#pragma once

// Core
#include "FileSystem/Core/IFileManager.h"
#include "FileSystem/Core/FileManager.h"

// IO
#include "FileSystem/IO/IFileIO.h"
#include "FileSystem/IO/IAsyncFileIO.h"
#include "FileSystem/IO/ThreadPoolAsyncFileIO.h"
#include "FileSystem/IO/BinaryFileIO.h"
#include "FileSystem/IO/JsonFileIO.h"

// Serialization
#include "FileSystem/Serialization/ISerializer.h"
#include "FileSystem/Serialization/BinarySerializer.h"

// Async Core
#include "FileSystem/Core/AsyncFileManager.h"

// Encryption
#include "FileSystem/Encryption/IEncryptionStrategy.h"
#include "FileSystem/Encryption/NoEncryption.h"
#include "FileSystem/Encryption/AesEncryption.h"