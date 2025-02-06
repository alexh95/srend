#include "platform.h"
#include "string.h"

inline static string StringC(char *Buffer, umm Size)
{
    string Result = {.Bytes.Char = Buffer, .Size = Size};
    return Result;
}

#define StringZ(Buffer) {.Bytes.Char = (Buffer), .Size = ArrayCount(Buffer) - 1}

#define StringCZ(Buffer) StringC((Buffer), ArrayCount(Buffer) - 1)

static string StringCFromTo(char *Buffer, umm FromInclusive, umm ToExclusive)
{
    int a = 0;
    Assert(FromInclusive <= ToExclusive);
    string Result = StringC(Buffer + FromInclusive, ToExclusive - FromInclusive);
    return Result;
}

inline static string StringFromTo(string S, umm FromInclusive, umm ToExclusive)
{
    Assert(FromInclusive <= S.Size && ToExclusive <= S.Size);
    string Result = StringCFromTo(S.Bytes.Char, FromInclusive, ToExclusive);
    return Result;
}

inline static string StringFrom(string S, umm FromInclusive)
{
    string Result = StringCFromTo(S.Bytes.Char, FromInclusive, S.Size);
    return Result;
}

inline static string StringTo(string S, umm ToExclusive)
{
    string Result = StringCFromTo(S.Bytes.Char, 0, ToExclusive);
    return Result;
}

umm StringCLength(char *Buffer)
{
    umm Result = 0;
    while (Buffer[Result] != 0)
    {
        ++Result;
    }
    return Result;
}

umm StringCCopyFromTo(char *Dst, umm DstOffset, char *Src, umm SrcFromInclusive, umm SrcToExclusive)
{
    Assert(SrcFromInclusive <= SrcToExclusive);
    umm Count = SrcToExclusive - SrcFromInclusive;
    for (umm Index = 0; Index < Count; ++Index)
    {
        umm DstIndex = Index + DstOffset;
        umm SrcIndex = Index + SrcFromInclusive;
        Dst[DstIndex] = Src[SrcIndex];
    }
    return DstOffset + Count;
}

inline static umm StringCopyFromTo(string Dst, umm DstOffset, string Src, umm SrcFromInclusive, umm SrcToExclusive)
{
    Assert(Dst.Size - DstOffset >= SrcToExclusive - SrcFromInclusive);
    umm Result = StringCCopyFromTo(Dst.Bytes.Char, DstOffset, Src.Bytes.Char, SrcFromInclusive, SrcToExclusive);
    return Result;
}

inline static umm StringCopyTo(string Dst, umm DstOffset, string Src, umm SrcToExclusive)
{
    umm Result = StringCCopyFromTo(Dst.Bytes.Char, DstOffset, Src.Bytes.Char, 0, SrcToExclusive);
    return Result;
}

inline static umm StringCopy(string Dst, umm DstOffset, string Src)
{
    umm Result = StringCopyFromTo(Dst, DstOffset, Src, 0, Src.Size);
    return Result;
}

umm StringCopyMultiple(string Dst, umm DstOffset, string *Srcs, u32 SrcCount)
{
    umm Result = DstOffset;
    for (u32 Index = 0; Index < SrcCount; ++Index)
    {
        Result = StringCopy(Dst, Result, Srcs[Index]);
    }
    return Result;
}

#define StringCopyS(Result, Dst, DstOffset, ...)\
{\
    string Srcs[] = {__VA_ARGS__};\
    u32 SrcCount = ArrayCount(Srcs);\
    Result = StringCopyMultiple(Dst, DstOffset, Srcs, SrcCount);\
}

inline static string ArenaPushString(memory_arena *Arena, umm Size)
{
    string Result = ArenaPushBuffer(Arena, Size);
    return Result;
}

inline static string ArenaPushStringCopy(memory_arena *Arena, string S)
{
    string Result = ArenaPushString(Arena, S.Size);
    StringCopy(Result, 0, S);
    return Result;
}

inline static string ArenaPushStringCopyFromTo(memory_arena *Arena, string S, umm From, umm To)
{
    Assert(To - From <= S.Size);
    string Result = ArenaPushString(Arena, To - From);
    StringCopyFromTo(Result, 0, S, From, To);
    return Result;
}

string_list StringSplitLines(memory_arena *Arena, string S)
{
    string_list Result = {};

    u32 LineCount = 1;
    for (umm Index = 0; Index < S.Size; ++Index)
    {
        if (S.Bytes.Char[Index] == '\n')
        {
            ++LineCount;
        }
    }
    Result.Strings = ArenaPushArray(Arena, string, LineCount);

    umm StartIndex = 0;
    for (umm Index = 0; Index < S.Size; ++Index)
    {
        if (S.Bytes.Char[Index] == '\n')
        {
            umm StringEndOffset = 0;
            if (Index > 0 && S.Bytes.Char[Index - 1] == '\r')
            {
                StringEndOffset = 1;
            }
            Result.Strings[Result.Count++] = StringCFromTo(S.Bytes.Char, StartIndex, Index - StringEndOffset);
            StartIndex = Index + 1;
        }
    }
    if (StartIndex < S.Size)
    {
        Result.Strings[Result.Count++] = StringCFromTo(S.Bytes.Char, StartIndex, S.Size);
    }

    return Result;
}

string_list StringSplitFromTo(memory_arena *Arena, string S, umm From, umm To, char Separator)
{
    string_list Result = {};

    u32 PartCount = 1;
    for (umm Index = From; Index < To; ++Index)
    {
        if (S.Bytes.Char[Index] == Separator)
        {
            ++PartCount;
        }
    }
    Result.Strings = ArenaPushArray(Arena, string, PartCount);

    umm StartIndex = From;
    for (umm Index = From; Index < To; ++Index)
    {
        if (S.Bytes.Char[Index] == Separator)
        {
            Result.Strings[Result.Count++] = StringFromTo(S, StartIndex, Index);
            StartIndex = Index + 1;
        }
    }
    Result.Strings[Result.Count++] = StringFromTo(S, StartIndex, To);

    return Result;
}

static inline string_list StringSplitFrom(memory_arena *Arena, string S, umm From, char Separator)
{
    string_list Result = StringSplitFromTo(Arena, S, From, S.Size, Separator);
    return Result;
}

static inline string_list StringSplit(memory_arena *Arena, string S, char Separator)
{
    string_list Result = StringSplitFromTo(Arena, S, 0, S.Size, Separator);
    return Result;
}

b32 StringCEquals(char *A, umm SizeA, char *B, umm SizeB)
{
    if (SizeA != SizeB)
    {
        return FALSE;
    }

    for (int Index = 0; Index < SizeA; ++Index)
    {
        if (A[Index] != B[Index])
        {
            return FALSE;
        }
    }

    return TRUE;
}

inline static b32 StringEquals(string A, string B)
{
    b32 Result = StringCEquals(A.Bytes.Char, A.Size, B.Bytes.Char, B.Size);
    return Result;
}

static string STRING_LF = StringZ("\n");
static string STRING_CRLF = StringZ("\r\n");

inline static b32 StringIsNewLine(string S)
{
    b32 Result = StringEquals(S, STRING_LF) || StringEquals(S, STRING_CRLF);
    return Result;
}

b32 StringStartsWith(string S, string Prefix)
{
    if (S.Size < Prefix.Size)
    {
        return FALSE;
    }

    b32 Result = StringCEquals(S.Bytes.Char, Prefix.Size, Prefix.Bytes.Char, Prefix.Size);
    return Result;
}

b32 StringEndsWith(string S, string Suffix)
{
    if (S.Size < Suffix.Size)
    {
        return FALSE;
    }

    umm Offset = S.Size - Suffix.Size;
    b32 Result = StringCEquals(S.Bytes.Char + Offset, Suffix.Size, Suffix.Bytes.Char, Suffix.Size);
    return Result;
}

s64 StringFirstIndexOf(string S, char Separator)
{
    for (u32 Index = 0; Index < S.Size; ++Index)
    {
        char Character = S.Bytes.Char[Index];
        if (Character == Separator)
        {
            return (s64)Index;
        }
    }
    return -1;
}

s64 StringLastIndexOf(string S, char Separator)
{
    for (u32 Index = S.Size - 1; Index >= 0; --Index)
    {
        char Character = S.Bytes.Char[Index];
        if (Character == Separator)
        {
            return (s64)Index;
        }
    }
    return -1;
}

string StringFromUMM(memory_arena *Arena, umm V)
{
    string Result = {};

    u64 Value = V;
    char Temp[20] = {};
    u32 Length = 0;
    while (Value > 0)
    {
        u64 Digit = Value % 10;
        Value /= 10;
        Temp[Length++] = (char)(48 + Digit);
    }

    if (Length == 0)
    {
        Temp[Length++] = '0';
    }

    Result = ArenaPushString(Arena, Length);
    for (u32 Index = 0; Index < Length; ++Index)
    {
        Result.Bytes.Char[Index] = Temp[Length - 1 - Index];
    }

    return Result;
}

s32 StringParseS32(string S)
{
    b32 IsNegative = FALSE;
    s32 Result = 0;
    for (umm Index = 0; Index < S.Size; ++Index)
    {
        c8 C = S.Bytes.Char[Index];
        if (C == '-')
        {
            IsNegative = TRUE;
        }
        else
        {
            s32 Digit = C - '0';
            Result = 10 * Result + Digit;
        }
    }
    return IsNegative ? -Result : Result;
}

f64 StringParseF64(string S)
{
    b32 Sign = 1.0;
    f64 WholePart = 0.0;
    f64 FractionalPart = 0.0;
    f64 FractionalDivisor = 1.0;
    b32 SeparatorMet = FALSE;
    for (umm Index = 0; Index < S.Size; ++Index)
    {
        c8 C = S.Bytes.Char[Index];
        if (C == '-')
        {
            Sign = -1.0;
        }
        else if (C == '.')
        {
            SeparatorMet = TRUE;
        }
        else
        {
            u32 Digit = C - '0';
            if (SeparatorMet)
            {
                FractionalPart = 10 * FractionalPart + Digit;
                FractionalDivisor *= 10;
            }
            else
            {
                WholePart = 10 * WholePart + Digit;
            }
        }
    }
    f64 Result = Sign * (WholePart + FractionalPart / FractionalDivisor);
    return Result;
}
