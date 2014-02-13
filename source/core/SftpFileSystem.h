//---------------------------------------------------------------------------
#ifndef SftpFileSystemH
#define SftpFileSystemH

#include <FileSystems.h>
//---------------------------------------------------------------------------
class TSFTPPacket;
class TOverwriteFileParams;
struct TSFTPSupport;
class TSecureShell;
//---------------------------------------------------------------------------
enum TSFTPOverwriteMode { omOverwrite, omAppend, omResume };
//---------------------------------------------------------------------------
class TSFTPFileSystem : public TCustomFileSystem
{
friend class TSFTPPacket;
friend class TSFTPQueue;
friend class TSFTPAsynchronousQueue;
friend class TSFTPUploadQueue;
friend class TSFTPDownloadQueue;
friend class TSFTPLoadFilesPropertiesQueue;
friend class TSFTPCalculateFilesChecksumQueue;
friend class TSFTPBusy;
public:
  __fastcall TSFTPFileSystem(TTerminal * ATerminal, TSecureShell * SecureShell);
  virtual __fastcall ~TSFTPFileSystem();

  virtual void __fastcall Open();
  virtual void __fastcall Close();
  virtual bool __fastcall GetActive();
  virtual void __fastcall CollectUsage();
  virtual void __fastcall Idle();
  virtual UnicodeString __fastcall AbsolutePath(UnicodeString Path, bool Local);
  virtual void __fastcall AnyCommand(const UnicodeString Command,
    TCaptureOutputEvent OutputEvent);
  virtual void __fastcall ChangeDirectory(const UnicodeString Directory);
  virtual void __fastcall CachedChangeDirectory(const UnicodeString Directory);
  virtual void __fastcall AnnounceFileListOperation();
  virtual void __fastcall ChangeFileProperties(const UnicodeString FileName,
    const TRemoteFile * File, const TRemoteProperties * Properties,
    TChmodSessionAction & Action);
  virtual bool __fastcall LoadFilesProperties(TStrings * FileList);
  virtual void __fastcall CalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum);
  virtual void __fastcall CopyToLocal(TStrings * FilesToCopy,
    const UnicodeString TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void __fastcall CopyToRemote(TStrings * FilesToCopy,
    const UnicodeString TargetDir, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress,
    TOnceDoneOperation & OnceDoneOperation);
  virtual void __fastcall CreateDirectory(const UnicodeString DirName);
  virtual void __fastcall CreateLink(const UnicodeString FileName, const UnicodeString PointTo, bool Symbolic);
  virtual void __fastcall DeleteFile(const UnicodeString FileName,
    const TRemoteFile * File, int Params, TRmSessionAction & Action);
  virtual void __fastcall CustomCommandOnFile(const UnicodeString FileName,
    const TRemoteFile * File, UnicodeString Command, int Params, TCaptureOutputEvent OutputEvent);
  virtual void __fastcall DoStartup();
  virtual void __fastcall HomeDirectory();
  virtual bool __fastcall IsCapable(int Capability) const;
  virtual void __fastcall LookupUsersGroups();
  virtual void __fastcall ReadCurrentDirectory();
  virtual void __fastcall ReadDirectory(TRemoteFileList * FileList);
  virtual void __fastcall ReadFile(const UnicodeString FileName,
    TRemoteFile *& File);
  virtual void __fastcall ReadSymlink(TRemoteFile * SymlinkFile,
    TRemoteFile *& File);
  virtual void __fastcall RenameFile(const UnicodeString FileName,
    const UnicodeString NewName);
  virtual void __fastcall CopyFile(const UnicodeString FileName,
    const UnicodeString NewName);
  virtual UnicodeString __fastcall FileUrl(const UnicodeString FileName);
  virtual TStrings * __fastcall GetFixedPaths();
  virtual void __fastcall SpaceAvailable(const UnicodeString Path,
    TSpaceAvailable & ASpaceAvailable);
  virtual const TSessionInfo & __fastcall GetSessionInfo();
  virtual const TFileSystemInfo & __fastcall GetFileSystemInfo(bool Retrieve);
  virtual bool __fastcall TemporaryTransferFile(const UnicodeString & FileName);
  virtual bool __fastcall GetStoredCredentialsTried();
  virtual UnicodeString __fastcall GetUserName();

protected:
  TSecureShell * FSecureShell;
  TFileSystemInfo FFileSystemInfo;
  bool FFileSystemInfoValid;
  int FVersion;
  UnicodeString FCurrentDirectory;
  UnicodeString FDirectoryToChangeTo;
  UnicodeString FHomeDirectory;
  AnsiString FEOL;
  TList * FPacketReservations;
  Variant FPacketNumbers;
  char FPreviousLoggedPacket;
  int FNotLoggedPackets;
  int FBusy;
  void * FBusyToken;
  bool FAvoidBusy;
  TStrings * FExtensions;
  TSFTPSupport * FSupport;
  bool FUtfStrings;
  bool FSignedTS;
  bool FOpenSSH;
  TStrings * FFixedPaths;
  unsigned long FMaxPacketSize;
  bool FSupportsStatVfsV2;

  void __fastcall SendCustomReadFile(TSFTPPacket * Packet, TSFTPPacket * Response,
    unsigned long Flags);
  void __fastcall CustomReadFile(const UnicodeString FileName,
    TRemoteFile *& File, unsigned char Type, TRemoteFile * ALinkedByFile = NULL,
    int AllowStatus = -1);
  virtual UnicodeString __fastcall GetCurrentDirectory();
  UnicodeString __fastcall GetHomeDirectory();
  unsigned long __fastcall GotStatusPacket(TSFTPPacket * Packet, int AllowStatus);
  bool __fastcall RemoteFileExists(const UnicodeString FullPath, TRemoteFile ** File = NULL);
  TRemoteFile * __fastcall LoadFile(TSFTPPacket * Packet,
    TRemoteFile * ALinkedByFile, const UnicodeString FileName,
    TRemoteFileList * TempFileList = NULL, bool Complete = true);
  void __fastcall LoadFile(TRemoteFile * File, TSFTPPacket * Packet,
    bool Complete = true);
  UnicodeString __fastcall LocalCanonify(const UnicodeString & Path);
  UnicodeString __fastcall Canonify(UnicodeString Path);
  UnicodeString __fastcall RealPath(const UnicodeString Path);
  UnicodeString __fastcall RealPath(const UnicodeString Path, const UnicodeString BaseDir);
  void __fastcall ReserveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response);
  int __fastcall ReceivePacket(TSFTPPacket * Packet, int ExpectedType = -1,
    int AllowStatus = -1);
  bool __fastcall PeekPacket();
  void __fastcall RemoveReservation(int Reservation);
  void __fastcall SendPacket(const TSFTPPacket * Packet);
  int __fastcall ReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response, int ExpectedType = -1, int AllowStatus = -1);
  int __fastcall SendPacketAndReceiveResponse(const TSFTPPacket * Packet,
    TSFTPPacket * Response, int ExpectedType = -1, int AllowStatus = -1);
  void __fastcall UnreserveResponse(TSFTPPacket * Response);
  void __fastcall TryOpenDirectory(const UnicodeString Directory);
  bool __fastcall SupportsExtension(const UnicodeString & Extension) const;
  void __fastcall ResetConnection();
  void __fastcall DoCalculateFilesChecksum(const UnicodeString & Alg,
    TStrings * FileList, TStrings * Checksums,
    TCalculatedChecksumEvent OnCalculatedChecksum,
    TFileOperationProgressType * OperationProgress, bool FirstLevel);
  void __fastcall DoDeleteFile(const UnicodeString FileName, unsigned char Type);

  void __fastcall SFTPSourceRobust(const UnicodeString FileName,
    const UnicodeString TargetDir, const TCopyParamType * CopyParam, int Params,
    TFileOperationProgressType * OperationProgress, unsigned int Flags);
  void __fastcall SFTPSource(const UnicodeString FileName,
    const UnicodeString TargetDir, const TCopyParamType * CopyParam, int Params,
    TFileOperationProgressType * OperationProgress, unsigned int Flags,
    TUploadSessionAction & Action, bool & ChildError);
  RawByteString __fastcall SFTPOpenRemoteFile(const UnicodeString & FileName,
    unsigned int OpenType, __int64 Size = -1);
  int __fastcall SFTPOpenRemote(void * AOpenParams, void * Param2);
  void __fastcall SFTPCloseRemote(const RawByteString Handle,
    const UnicodeString FileName, TFileOperationProgressType * OperationProgress,
    bool TransferFinished, bool Request, TSFTPPacket * Packet);
  void __fastcall SFTPDirectorySource(const UnicodeString DirectoryName,
    const UnicodeString TargetDir, int Attrs, const TCopyParamType * CopyParam,
    int Params, TFileOperationProgressType * OperationProgress, unsigned int Flags);
  void __fastcall SFTPConfirmOverwrite(UnicodeString & FileName,
    const TCopyParamType * CopyParam, int Params, TFileOperationProgressType * OperationProgress,
    TSFTPOverwriteMode & Mode, const TOverwriteFileParams * FileParams);
  bool SFTPConfirmResume(const UnicodeString DestFileName, bool PartialBiggerThanSource,
    TFileOperationProgressType * OperationProgress);
  void __fastcall SFTPSinkRobust(const UnicodeString FileName,
    const TRemoteFile * File, const UnicodeString TargetDir,
    const TCopyParamType * CopyParam, int Params,
    TFileOperationProgressType * OperationProgress, unsigned int Flags);
  void __fastcall SFTPSink(const UnicodeString FileName,
    const TRemoteFile * File, const UnicodeString TargetDir,
    const TCopyParamType * CopyParam, int Params,
    TFileOperationProgressType * OperationProgress, unsigned int Flags,
    TDownloadSessionAction & Action, bool & ChildError);
  void __fastcall SFTPSinkFile(UnicodeString FileName,
    const TRemoteFile * File, void * Param);
  char * __fastcall GetEOL() const;
  inline void __fastcall BusyStart();
  inline void __fastcall BusyEnd();
  inline unsigned long __fastcall TransferBlockSize(unsigned long Overhead,
    TFileOperationProgressType * OperationProgress, unsigned long MaxPacketSize = 0);
  inline unsigned long __fastcall UploadBlockSize(const RawByteString & Handle,
    TFileOperationProgressType * OperationProgress);
  inline unsigned long __fastcall DownloadBlockSize(
    TFileOperationProgressType * OperationProgress);
  inline int __fastcall PacketLength(unsigned char * LenBuf, int ExpectedType);
};
//---------------------------------------------------------------------------
#endif // SftpFileSystemH
