#include "Math/Vector.isph"

struct FStaticAcceleratedManagedSignificanceInfo
{
    // Basic info
    FVector3f CachedLocation;
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
        // Load all input values into local variables to minimize gather operations
        const FVector3f v2 = Data[i].CachedLocation;
        const float SignificanceDistanceSquare = Data[i].SignificanceDistanceSquare;
        const float OldSignifiance = Data[i].CurrentSignificance;
        const int8 OldCurrentTickStage = Data[i].CurrentTickStage;
        const int8 NumTickStages = Data[i].NumTickStages;
                
        float A = square(v2.V[0] - v1.V[0]);
        float B = square(v2.V[1] - v1.V[1]);
        float C = square(v2.V[2] - v1.V[2]);
            
        const float DistSquared = A+B+C;

        // > 0 is relevant
        const float NewSignifiance = min(1.f - max((DistSquared / SignificanceDistanceSquare), 0.0f),1.0);

        // Compute new values
        const int8 NewIsSignificant = NewSignifiance >= 0.01f;
        const int8 NewWasSignificant = OldSignifiance == -1.f ? -1 : OldSignifiance >= 0.01f;
        const int8 NewCurrentTickStage = NumTickStages * NewSignifiance;
        const int8 NewDidChangeTickState = OldCurrentTickStage != NewCurrentTickStage;

        // Store all output values at once to minimize scatter operations
        Data[i].CurrentSignificance = NewSignifiance;
        Data[i].bIsSignificant = NewIsSignificant;
        Data[i].bWasSignificant = NewWasSignificant;
        Data[i].bDidChangeTickState = NewDidChangeTickState;
        Data[i].CurrentTickStage = NewCurrentTickStage;
    }
};