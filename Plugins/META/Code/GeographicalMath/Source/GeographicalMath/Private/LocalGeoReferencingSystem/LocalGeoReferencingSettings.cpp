#include "LocalGeoReferencingSystem/LocalGeoReferencingSettings.h"

ULocalGeoReferencingSettings* ULocalGeoReferencingSettings::Get()
{
        return CastChecked<ULocalGeoReferencingSettings>(StaticClass()->GetDefaultObject());
}
