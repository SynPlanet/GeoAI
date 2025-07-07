// META all rights received (c)

#include "RulerSelection.h"

void URulerSelection::SetPolygonVisibility(bool bIsVisible)
{
        if (PolygonWidget.IsValid())
        {
                PolygonWidget->SetPolygonVisibility(bIsVisible);
        }
}

void URulerSelection::NativeConstruct()
{
        Super::NativeConstruct();
        CreatePolygonSelection(PolygonColor);
}

void URulerSelection::CreatePolygonSelection(FLinearColor Color)
{
        if (GEngine && GEngine->GameViewport)
        {
                SAssignNew(PolygonWidget, SPolygonWidget).FillColor(Color);

                if (PolygonWidget.IsValid())
                {
                        GEngine->GameViewport->AddViewportWidgetContent(PolygonWidget.ToSharedRef(), -200);
                        PolygonWidget->SetPolygonPoints(PolygonPoints);
                }
        }
}

void URulerSelection::PolygonSelectionUpdatePoints(TArray<FVector> Points) const
{
        if (!PolygonWidget.IsValid())
        {
                return;
        }

        PolygonWidget->SetPolygonPoints(Points);
}

void URulerSelection::NativeDestruct()
{
        Super::NativeDestruct();

        if (GEngine && GEngine->GameViewport && PolygonWidget.IsValid())
        {
                GEngine->GameViewport->RemoveViewportWidgetContent(PolygonWidget.ToSharedRef());
                PolygonWidget.Reset();
        }
}

void URulerSelection::SetPolygonPoints(const TArray<FVector>& NewPoints)
{
        PolygonPoints = NewPoints;
}

const TArray<FVector>& URulerSelection::GetPolygonPoints()
{
        return PolygonPoints;
}