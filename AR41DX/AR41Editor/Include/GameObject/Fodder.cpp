#include "Fodder.h"
#include "Component/StaticMeshComponent.h"
#include "Component/AnimationMeshComponent.h"
#include "Component/StaticMeshComponent.h"
#include "Component/ColliderCube.h"
#include "Component/ColliderOBB3D.h"
#include "Component/RigidBody.h"
#include "Input.h"
#include "Scene/Scene.h"
#include "Scene/SceneManager.h"
#include "Scene/CameraManager.h"
#include "Device.h"
#include "Resource/Material/Material.h"
#include "Animation/Animation.h"
#include "Engine.h"
#include "Player.h"
#include "FodderDebris.h"


CFodder::CFodder() :
    m_DetectOn(false),
    m_AttackOn(false),
    m_WeaponAttack(false)
{
    SetTypeID<CFodder>();

    m_ObjectTypeName = "Fodder";
}

CFodder::CFodder(const CFodder& Obj)
{
    m_DetectArea = (CColliderOBB3D*)FindComponent("DetectArea");
    m_AttackArea = (CColliderOBB3D*)FindComponent("AttackArea");
    m_BodyCube = (CColliderOBB3D*)FindComponent("BodyCube");
    m_WeaponCube = (CColliderOBB3D*)FindComponent("WeaponCube");
    m_Mesh = (CAnimationMeshComponent*)FindComponent("Mesh");
    m_Rigid = (CRigidBody*)FindComponent("Rigid");
}

CFodder::~CFodder()
{
}

void CFodder::Start()
{
    CMonster::Start();

    // 테스트용 키세팅
    CInput::GetInst()->AddBindFunction<CFodder>("F", Input_Type::Down, this, &CFodder::Debris, m_Scene);

    // 탐지범위에 플레이어가 들어올 시 Notice 애니메이션 후 Walk 로 변경.
    //m_Animation->SetCurrentEndFunction("Fodder_Notice", this, &CFodder::Walk);
    //m_Animation->SetCurrentEndFunction("Fodder_Dead", this, &CFodder::Debris);
    m_Animation->SetCurrentEndFunction("Fodder_Attack", this, &CFodder::WeaponAttackOn);
    m_Animation->AddCurrentNotify<CFodder>("Fodder_Attack", "Fodder_Attack", 14, this, &CFodder::AttackSound);

    m_DetectArea->SetCollisionCallback<CFodder>(ECollision_Result::Collision, this, &CFodder::Collision_Detect_ChaseOn);
    m_DetectArea->SetCollisionCallback<CFodder>(ECollision_Result::Release, this, &CFodder::Release_Detect_ChaseOff);
    m_AttackArea->SetCollisionCallback<CFodder>(ECollision_Result::Collision, this, &CFodder::Collision_AttackOn);
    m_AttackArea->SetCollisionCallback<CFodder>(ECollision_Result::Release, this, &CFodder::Release_AttackOff);
    m_BodyCube->SetCollisionCallback<CFodder>(ECollision_Result::Collision, this, &CFodder::Collision_Body);
    m_WeaponCube->SetCollisionCallback<CFodder>(ECollision_Result::Collision, this, &CFodder::Collision_WeaponAttack);
    m_WeaponCube->SetCollisionCallback<CFodder>(ECollision_Result::Release, this, &CFodder::Release_WeaponAttackOff);

    // m_FodderBT->Start();
}

bool CFodder::Init()
{
    if (!CMonster::Init())
    {
        return false;
    }

    m_Mesh = CreateComponent<CAnimationMeshComponent>("Mesh");


    m_DetectArea = CreateComponent<CColliderOBB3D>("DetectArea");
    m_AttackArea = CreateComponent<CColliderOBB3D>("AttackArea");
    m_BodyCube = CreateComponent<CColliderOBB3D>("BodyCube");
    m_WeaponCube = CreateComponent<CColliderOBB3D>("WeaponCube");
    m_Rigid = CreateComponent<CRigidBody>("Rigid");

    SetRootComponent(m_Mesh);

    m_Mesh->SetMesh("Fodder");
    m_Mesh->SetWorldPosition(100.f, 50.f, 150.f);
    m_Mesh->AddChild(m_DetectArea);
    m_Mesh->AddChild(m_AttackArea);
    m_Mesh->AddChild(m_BodyCube);
    m_Mesh->AddChild(m_WeaponCube);
    m_Mesh->AddChild(m_Rigid);


    m_DetectArea->SetCollisionProfile("DetectArea");
    m_DetectArea->SetBoxHalfSize(800.f, 400.f, 800.f);
    m_DetectArea->SetRelativePosition(0.f, 400.f);

    m_AttackArea->SetCollisionProfile("DetectArea");
    m_AttackArea->SetBoxHalfSize(300.f, 50.f, 300.f);
    m_AttackArea->SetRelativePosition(0.f, 70.f);

    m_BodyCube->SetCollisionProfile("Monster");
    m_BodyCube->SetRelativePosition(0.f, 100.f, 0.f);

    m_BodyCube->SetBoxHalfSize(30.f, 100.f, 40.f);

    m_WeaponCube->SetCollisionProfile("MonsterAttack");
    m_WeaponCube->SetRelativePosition(50.f, 80.f, -100.f);
    m_WeaponCube->SetBoxHalfSize(50.f, 80.f, 50.f);
    m_WeaponCube->SetInheritRotX(true);
    m_WeaponCube->SetInheritRotY(true);
    m_WeaponCube->SetInheritRotZ(true);

    m_Animation = m_Mesh->SetAnimation<CAnimation>("FodderAnimation");

    m_Animation->AddAnimation("Fodder_Walk", "Fodder_Walk", 1.f, 1.f, true);
    m_Animation->AddAnimation("Fodder_Attack", "Fodder_Attack", 1.f, 1.5f, true);
    m_Animation->AddAnimation("Fodder_Notice", "Fodder_Notice", 1.f, 1.f, false);
    m_Animation->AddAnimation("Fodder_Dead", "Fodder_Dead", 1.f, 1.f, false);

    // m_FodderBT = new CFodderBT;

    //m_Rigid->SetGravity(true);

    return true;
}

void CFodder::Update(float DeltaTime)
{
    CMonster::Update(DeltaTime);

    // 탐지범위내에 있으면 
    if (m_DetectOn)
    {
        // m_Animation->ChangeAnimation("Fodder_Notice");

        Detect_Chase();

        // 공격범위내에 있으면
        if (m_AttackOn)
        {
            m_Animation->ChangeAnimation("Fodder_Attack");

            if (m_Animation->GetCurrentAnimationName() == "Fodder_Attack")
            {
                float FodderSpeed = GetMoveSpeed();

                FodderSpeed = 0.f;
            }
        }
    }

    //AddWorldPositionX(300.f * DeltaTime);

    // m_FodderBT->Run(this);
}

void CFodder::PostUpdate(float DeltaTime)
{
    CMonster::PostUpdate(DeltaTime);
}

CFodder* CFodder::Clone() const
{
    return new CFodder(*this);
}

void CFodder::Save(FILE* File)
{
    CMonster::Save(File);
}

void CFodder::Load(FILE* File)
{
    CMonster::Load(File);
}

void CFodder::Walk()
{
    m_Animation->ChangeAnimation("Fodder_Walk");
}

void CFodder::Detect_Chase()
{
    CPlayer* Player = (CPlayer*)m_Scene->GetPlayerObject();

    if (!Player)
        return;

    //Vector3 FodderPos;

    /*if (CSceneManager::GetInst()->GetScene()->FindObject("Fodder"))
    {*/
    //Vector3 FodderPos = CSceneManager::GetInst()->GetScene()->FindObject("Fodder")->GetWorldPos();
    //}
    Vector3 FodderPos = GetWorldPos();

    Vector3 PlayerPos = CSceneManager::GetInst()->GetScene()->GetPlayerObject()->GetWorldPos();

    Vector3 Dir = PlayerPos - FodderPos;

    Dir.y = 0.f; // Y축으로 이동 불가. 

    Dir.Normalize();

    float FodderSpeed = GetMoveSpeed();

    // m_MoveSpeed는 Monster 클래스에서 100.f로 되어있다. 속도 2배 빠르게.
    AddWorldPosition(Dir * 2 * FodderSpeed * g_DeltaTime);

    // 플레이어 바라보게끔.
    float Degree = atan2(GetWorldPos().z - PlayerPos.z, GetWorldPos().x - PlayerPos.x);
    Degree = fabs(Degree * 180.f / PI - 180.f) - 90.f;

    SetWorldRotationY(Degree);
}

void CFodder::Attack()
{
    m_Animation->ChangeAnimation("Fodder_Attack");
}

void CFodder::WeaponAttackOn()
{
    m_WeaponAttack = true;
}

void CFodder::Dead()
{
    m_Mesh->AddWorldPositionZ(150.f * g_DeltaTime);
    m_Rigid->SetVelocity(0.f, 250.f, 200.f);
    m_Rigid->AddForce(0.f, 100.f, 500.f);
    m_Animation->ChangeAnimation("Fodder_Dead");

    SetMoveSpeed(0.f);
}

void CFodder::Debris()
{
    if (!m_Scene->FindObject("FodderDebris"))
    {
        Vector3 FodderPos = GetWorldPos();

        m_Mesh->Destroy();
        m_Mesh->ClearMaterial();

        CFodderDebris* Debris = m_Scene->CreateObject<CFodderDebris>("FodderDebris");
        Debris->SetWorldPosition(FodderPos.x, FodderPos.y + 30.f, FodderPos.z);
    }
}

void CFodder::AttackSound()
{
    CResourceManager::GetInst()->SoundPlay("Fodder_Attack");
    CResourceManager::GetInst()->SetVolume(10.f);
}

void CFodder::Collision_Detect_ChaseOn(const CollisionResult& result)
{
    std::string Name = result.Dest->GetCollisionProfile()->Name;

    if (Name == "Player")
    {
        m_DetectOn = true;
    }
}

void CFodder::Release_Detect_ChaseOff(const CollisionResult& result)
{
    if (result.Dest != nullptr)
    {
        std::string Name = result.Dest->GetCollisionProfile()->Name;

        if (Name == "Player")
        {
            m_DetectOn = false;
        }
    }
}

void CFodder::Collision_AttackOn(const CollisionResult& result)
{
    std::string Name = result.Dest->GetCollisionProfile()->Name;

    if (Name == "Player")
    {
        m_AttackOn = true;

    }
}

void CFodder::Release_AttackOff(const CollisionResult& result)
{
    // 플레이어가 사라졌을 때 대비
    if (result.Dest != nullptr)
    {
        std::string Name = result.Dest->GetCollisionProfile()->Name;

        if (Name == "Player")
        {
            m_AttackOn = false;
            m_Animation->ChangeAnimation("Fodder_Walk");
        }
    }
}

void CFodder::Collision_Body(const CollisionResult& result)
{
    // 데미지 받는 것.
    std::string Name = result.Dest->GetCollisionProfile()->Name;

    if (Name == "Player")
    {
        result.Dest->GetOwner()->InflictDamage(1);
    }

    if (Name == "PlayerAttack")
    {

        CResourceManager::GetInst()->SoundPlay("Ham_Hit");
        CResourceManager::GetInst()->SetVolume(30.f);

        Debris();
    }
}

void CFodder::Collision_WeaponAttack(const CollisionResult& result)
{
    std::string Name = result.Dest->GetCollisionProfile()->Name;


    if (Name == "Player" && m_WeaponAttack)
    {

        result.Dest->GetOwner()->InflictDamage(1);
    }
}

void CFodder::Release_WeaponAttackOff(const CollisionResult& result)
{
    m_WeaponAttack = false;
}

void CFodder::MoveFront()
{
    m_WeaponCube->AddWorldPositionY(10.f);
}

void CFodder::MoveBack()
{
    m_WeaponCube->AddWorldPositionY(-10.f);
}