// META all rights received (c)

#include "RulerLines.h"

void URulerLines::SetPoints(const TArray<FVector>& NewPoints)
{
        Points = NewPoints;
}

const TArray<FVector>& URulerLines::GetPoints()
{
        return Points;
}

void URulerLines::ChangeLinesColor(FLinearColor NewColor)
{
        if (RulerLinesColor != NewColor)
        {
                RulerLinesColor = NewColor;
        }
}
