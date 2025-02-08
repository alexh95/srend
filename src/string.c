#include "platform.h"
#include "string.h"

static inline string StringC(c8 *Buffer, umm Size)
{
    string Result = {.Bytes.Char = Buffer, .Size = Size};
    return Result;
}

#define StringZ(Buffer) {.Bytes.Char = (Buffer), .Size = ArrayCount(Buffer) - 1}

#define StringCZ(Buffer) StringC((Buffer), ArrayCount(Buffer) - 1)

static inline string StringCFromTo(c8 *Buffer, umm FromInclusive, umm ToExclusive)
{
    Assert(FromInclusive <= ToExclusive);
    string Result = StringC(Buffer + FromInclusive, ToExclusive - FromInclusive);
    return Result;
}

static inline string StringFromTo(string S, umm FromInclusive, umm ToExclusive)
{
    Assert(FromInclusive <= S.Size && ToExclusive <= S.Size);
    string Result = StringCFromTo(S.Bytes.Char, FromInclusive, ToExclusive);
    return Result;
}

static inline string StringFrom(string S, umm FromInclusive)
{
    string Result = StringCFromTo(S.Bytes.Char, FromInclusive, S.Size);
    return Result;
}

static inline string StringTo(string S, umm ToExclusive)
{
    string Result = StringCFromTo(S.Bytes.Char, 0, ToExclusive);
    return Result;
}

umm StringCLength(c8 *Buffer)
{
    umm Result = 0;
    while (Buffer[Result] != 0)
    {
        ++Result;
    }
    return Result;
}

umm StringCCopyFromTo(c8 *Dst, umm DstOffset, c8 *Src, umm SrcFromInclusive, umm SrcToExclusive)
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

static inline umm StringCopyFromTo(string Dst, umm DstOffset, string Src, umm SrcFromInclusive, umm SrcToExclusive)
{
    Assert(Dst.Size - DstOffset >= SrcToExclusive - SrcFromInclusive);
    umm Result = StringCCopyFromTo(Dst.Bytes.Char, DstOffset, Src.Bytes.Char, SrcFromInclusive, SrcToExclusive);
    return Result;
}

static inline umm StringCopyTo(string Dst, umm DstOffset, string Src, umm SrcToExclusive)
{
    umm Result = StringCCopyFromTo(Dst.Bytes.Char, DstOffset, Src.Bytes.Char, 0, SrcToExclusive);
    return Result;
}

static inline umm StringCopy(string Dst, umm DstOffset, string Src)
{
    umm Result = StringCopyFromTo(Dst, DstOffset, Src, 0, Src.Size);
    return Result;
}

static inline umm StringCopyMultiple(string Dst, umm DstOffset, string *Srcs, u32 SrcCount)
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

static inline string ArenaPushString(memory_arena *Arena, umm Size)
{
    string Result = ArenaPushBuffer(Arena, Size);
    return Result;
}

static inline string ArenaPushStringCopy(memory_arena *Arena, string S)
{
    string Result = ArenaPushString(Arena, S.Size);
    StringCopy(Result, 0, S);
    return Result;
}

static inline string ArenaPushStringCopyFromTo(memory_arena *Arena, string S, umm From, umm To)
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

string_list StringSplitFromTo(memory_arena *Arena, string S, umm From, umm To, c8 Separator)
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

static inline string_list StringSplitFrom(memory_arena *Arena, string S, umm From, c8 Separator)
{
    string_list Result = StringSplitFromTo(Arena, S, From, S.Size, Separator);
    return Result;
}

static inline string_list StringSplit(memory_arena *Arena, string S, c8 Separator)
{
    string_list Result = StringSplitFromTo(Arena, S, 0, S.Size, Separator);
    return Result;
}

b32 StringCEquals(c8 *A, umm SizeA, c8 *B, umm SizeB)
{
    if (SizeA != SizeB)
    {
        return FALSE;
    }

    for (umm Index = 0; Index < SizeA; ++Index)
    {
        if (A[Index] != B[Index])
        {
            return FALSE;
        }
    }

    return TRUE;
}

static inline b32 StringEquals(string A, string B)
{
    b32 Result = StringCEquals(A.Bytes.Char, A.Size, B.Bytes.Char, B.Size);
    return Result;
}

static string STRING_LF = StringZ("\n");
static string STRING_CRLF = StringZ("\r\n");

static inline b32 StringIsNewLine(string S)
{
    b32 Result = StringEquals(S, STRING_LF) || StringEquals(S, STRING_CRLF);
    return Result;
}

static b32 StringStartsWith(string S, string Prefix)
{
    if (S.Size < Prefix.Size)
    {
        return FALSE;
    }

    b32 Result = StringCEquals(S.Bytes.Char, Prefix.Size, Prefix.Bytes.Char, Prefix.Size);
    return Result;
}

static b32 StringEndsWith(string S, string Suffix)
{
    if (S.Size < Suffix.Size)
    {
        return FALSE;
    }

    umm Offset = S.Size - Suffix.Size;
    b32 Result = StringCEquals(S.Bytes.Char + Offset, Suffix.Size, Suffix.Bytes.Char, Suffix.Size);
    return Result;
}

static s64 StringFirstIndexOf(string S, c8 Separator)
{
    for (u32 Index = 0; Index < S.Size; ++Index)
    {
        c8 Character = S.Bytes.Char[Index];
        if (Character == Separator)
        {
            return (s64)Index;
        }
    }
    return -1;
}

static s64 StringLastIndexOf(string S, c8 Separator)
{
    for (s32 Index = (s32)S.Size - 1; Index >= 0; --Index)
    {
        char Character = S.Bytes.Char[Index];
        if (Character == Separator)
        {
            return (s64)Index;
        }
    }
    return -1;
}

static string StringFromUMM(memory_arena *Arena, umm V)
{
    string Result = {};

    u64 Value = V;
    c8 Temp[20] = {};
    u32 Length = 0;
    while (Value > 0)
    {
        u64 Digit = Value % 10;
        Value /= 10;
        Temp[Length++] = (c8)(48 + Digit);
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

static s32 StringParseS32(string S)
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

static f64 StringParseF64(string S)
{
    f64 Sign = 1.0;
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

static umm StringCountOccurrenceFromTo(string S, umm From, umm To, c8 C)
{
    umm Result = 0;
    for (umm Index = From; Index < To; ++Index)
    {
        c8 Character = S.Bytes.Char[Index];
        if (C == Character)
        {
            ++Result;
        }
    }
    return Result;
}

static inline umm StringCountOccurrenceFrom(string S, umm From, c8 C)
{
    umm Result = StringCountOccurrenceFromTo(S, From, S.Size, C);
    return Result;
}

static inline umm StringCountOccurrenceTo(string S, umm To, c8 C)
{
    umm Result = StringCountOccurrenceFromTo(S, 0, To, C);
    return Result;
}

static inline umm StringCountOccurrence(string S, c8 C)
{
    umm Result = StringCountOccurrenceFromTo(S, 0, S.Size, C);
    return Result;
}
