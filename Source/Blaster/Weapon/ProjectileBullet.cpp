// Copyright Nathan Guerin


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (AController* OwnerController = OwnerCharacter->GetController())
		{
			UGameplayStatics::ApplyDamage(OtherActor, BaseDamage, OwnerController, this, UDamageType::StaticClass());
		}
	}
	
	Super::OnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
