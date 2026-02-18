#include "Math/Vector.isph"

struct FStaticAcceleratedManagedSignificanceInfo
{
    // Basic info
    FVector3f Location;
    float SignificanceDistanceSquare;
    int8 NumTickStages;
    float TickRateResponseExp;

    // Updated data.
	int8 CurrentTickStage;	
	float CurrentSignificance;
    
    int8 bIsSignificant;
    int8 bWasSignificant;
    int8 bDidChangeTickState;

    // non ispc data
    bool bSupportsTickRateDilation;
};

inline float square(const float& a)
{
    return a * a;
}

export void ComputeSignififance( const uniform FVector3f ViewPoints[],
                                 const uniform int32 NumViewports,
                                 uniform FStaticAcceleratedManagedSignificanceInfo uniform Data[],
                                 const uniform int32 Begin,
                                 const uniform int32 End)
{
    const varying FVector3f v1 = ViewPoints[0];

    foreach(i = Begin ... End)
    {    
        // Load all required struct members into local variables to minimize gather operations
        const FVector3f v2 = Data[i].Location;
        const float SignificanceDistanceSquare = Data[i].SignificanceDistanceSquare;
        const int8 NumTickStages = Data[i].NumTickStages;
        const float OldSignificance = Data[i].CurrentSignificance;
        const int8 OldCurrentTickStage = Data[i].CurrentTickStage;
                
        // Perform all computations on local variables
        float A = square(v2.V[0] - v1.V[0]);
        float B = square(v2.V[1] - v1.V[1]);
        float C = square(v2.V[2] - v1.V[2]);
            
        const float DistSquared = A+B+C;

        // > 0 is relevant
        const float NewSignificance = min(1.f - max((DistSquared / SignificanceDistanceSquare), 0.0f),1.0);

        // Compute all new values
        const int8 NewbIsSignificant = NewSignificance >= 0.01f;
        const int8 NewbWasSignificant = OldSignificance == -1.f ? -1 : OldSignificance >= 0.01f;
        const int8 NewCurrentTickStage = (int8)(NumTickStages * NewSignificance);
        const int8 NewbDidChangeTickState = OldCurrentTickStage != NewCurrentTickStage;

        // Write all values back in one batch to minimize scatter operations
        Data[i].CurrentSignificance = NewSignificance; 
        Data[i].bIsSignificant = NewbIsSignificant;
        Data[i].bWasSignificant = NewbWasSignificant;
        Data[i].CurrentTickStage = NewCurrentTickStage;
        Data[i].bDidChangeTickState = NewbDidChangeTickState;
    }
};