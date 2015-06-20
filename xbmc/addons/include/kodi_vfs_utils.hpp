/*
 *      Copyright (C) 2013-2015 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

/*!
 * @file kodi_vfs_utils.hpp  C++ wrappers for Kodi's VFS operations
 */

#include "libXBMC_addon.h"

#include <cstddef>
#include <stdint.h>
#include <stdio.h>
#include <string>

/* Platform dependent path separator */
#ifndef PATH_SEPARATOR_CHAR
  #if (defined(_WIN32) || defined(_WIN64))
    #define PATH_SEPARATOR_CHAR '\\'
  #else
    #define PATH_SEPARATOR_CHAR '/'
  #endif
#endif


namespace ADDON
{

class VFSFile
{
public:
  /*!
   * @brief Construct a new, unopened file
   * @param XBMC The libXBMC helper instance. Must remain valid during the lifetime of this file.
   */
  VFSFile(ADDON::CHelper_libXBMC_addon* XBMC) : m_XBMC(XBMC), m_pFile(NULL) { }

  /*!
   * @brief Close() is called from the destructor, so explicitly closing the file isn't required
   */
  virtual ~VFSFile() { Close(); }

  /*!
   * @brief Open a file via XBMC's CFile
   * @param strFileName The filename to open
   * @param flags The flags to pass. Documented in XBMC's File.h
   * @return True on success or false on failure
   */
  bool Open(const std::string &strFileName, unsigned int flags = 0)
  {
    if (!m_XBMC)
      return false;
    Close();
    m_pFile = m_XBMC->OpenFile(strFileName.c_str(), flags);
    return m_pFile != NULL;
  }

  /*!
   * @brief Open a file via XBMC's CFile in write mode. The file (and all subdirectories)
   *        will be created if necessary.
   * @param strFileName The filename to open
   * @param bOverWrite True to overwrite, false otherwise
   * @return True on success or false on failure
   */
  bool OpenForWrite(const std::string& strFileName, bool bOverWrite = false)
  {
    if (!m_XBMC)
      return false;
    Close();

    // Try to open the file. If it fails, check if we need to create the directory first
    // This way we avoid checking if the directory exists every time
    m_pFile = m_XBMC->OpenFileForWrite(strFileName.c_str(), bOverWrite);
    if (!m_pFile)
    {
      std::string cacheDirectory = GetDirectoryName(strFileName);
      if (m_XBMC->DirectoryExists(cacheDirectory.c_str()) || m_XBMC->CreateDirectory(cacheDirectory.c_str()))
        m_pFile = m_XBMC->OpenFileForWrite(strFileName.c_str(), bOverWrite);
    }
    return m_pFile != NULL;
  }

  /*!
   * @brief Read from an open file
   * @param lpBuffer The buffer to store the data in
   * @param uiBufferSize The size of the buffer
   * @return Number of bytes read
   */
  int64_t Read(void *lpBuffer, uint64_t uiBufferSize)
  {
    if (!m_pFile)
      return 0;
    return m_XBMC->ReadFile(m_pFile, lpBuffer, uiBufferSize);
  }

  /*!
   * @brief Read a string from an open file
   * @param strLine The buffer to store the data in
   * @return True when a line was read, false otherwise
   */
  bool ReadLine(std::string &strLine)
  {
    strLine.clear();
    if (!m_pFile)
      return false;
    // TODO: Read 1024 chars into buffer. If file position advanced that many
    // chars, we didn't hit a newline. Otherwise, if file position is 1 or 2
    // past the number of bytes read, we read (and skipped) a newline sequence.
    char buffer[1025];
    if (m_XBMC->ReadFileString(m_pFile, buffer, sizeof(buffer))) // will read 1024 bytes
    {
      strLine = buffer;
      return !strLine.empty();
    }
    return false;
  }

  /*!
   * @brief Write to a file opened in write mode
   * @param lpBuffer The data to write
   * @param uiBufferSize Size of the data to write
   * @return The number of bytes written
   */
  int64_t Write(const void* lpBuffer, uint64_t uiBufferSize)
  {
    if (!m_pFile)
      return 0;
    return m_XBMC->WriteFile(m_pFile, lpBuffer, uiBufferSize);
  }

  /*!
   * @brief Flush buffered data
   */
  void Flush()
  {
    if (!m_pFile)
      return;
    m_XBMC->FlushFile(m_pFile);
  }

  /*!
   * @brief Seek in an open file
   * @param iFilePosition The new position
   * @param iWhence Seek argument, see stdio.h for possible values
   * @return The new position
   */
  int64_t Seek(int64_t iFilePosition, int iWhence = SEEK_SET)
  {
    if (!m_pFile)
      return 0;
    return m_XBMC->SeekFile(m_pFile, iFilePosition, iWhence);
  }

  /*!
   * @brief Truncate a file to the requested size
   * @param iSize The new max size
   * @return The new size
   */
  int64_t Truncate(uint64_t iSize)
  {
    if (!m_pFile)
      return -1;
    return m_XBMC->TruncateFile(m_pFile, iSize);
  }

  /*!
   * @brief The current position in an open file
   * @return The current position
   */
  int64_t GetPosition() const
  {
    if (!m_pFile)
      return -1;
    return m_XBMC->GetFilePosition(m_pFile);
  }

  /*!
   * @brief Get the file size of an open file
   * @return The total file size
   */
  int64_t GetLength()
  {
    if (!m_pFile)
      return 0;
    return m_XBMC->GetFileLength(m_pFile);
  }

  /*!
   * @brief Close an open file
   */
  void Close()
  {
    if (!m_pFile)
      return;
    m_XBMC->CloseFile(m_pFile);
    m_pFile = NULL;
  }

  /*!
   * @brief Get the minimum size that can be read from the open file. For example,
   *        CDROM files in which access could be sector-based. It can also be
   *        used to indicate a file system is non-buffered but accepts any read
   *        size, in which case GetChunkSize() should return the value 1.
   * @return The chunk size
   */
  int GetChunkSize()
  {
    if (!m_pFile)
      return 0;
    return m_XBMC->GetFileChunkSize(m_pFile);
  }

  /*!
   * @brief Return a size aligned to the chunk size at least as large as the chunk size.
   * @param chunk The chunk size
   * @param minimum The minimum size (or maybe the minimum number of chunks?)
   * @return The aligned size
   */
  static unsigned int GetChunkSize(unsigned int chunk, unsigned int minimum)
  {
    if (chunk)
      return chunk * ((minimum + chunk - 1) / chunk);
    else
      return minimum;
  }

  static std::string GetFileName(const std::string &path, char separator = PATH_SEPARATOR_CHAR)
  {
    size_t pos = path.find_last_of(separator);
    return path.substr(pos + 1);
  }

  static std::string GetDirectoryName(const std::string &path, char separator = PATH_SEPARATOR_CHAR)
  {
    size_t pos = path.find_last_of(separator);
    return path.substr(0, pos);
  }

private:
  ADDON::CHelper_libXBMC_addon* m_XBMC;
  void *m_pFile;
};

}
