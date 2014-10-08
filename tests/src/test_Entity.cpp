#include <gmock/gmock.h>
#include <ontology/Ontology.hpp>

#define NAME Entity

using namespace Ontology;

// ----------------------------------------------------------------------------
// test fixtures
// ----------------------------------------------------------------------------

struct TestComponent : public Component
{
    TestComponent(int x, int y) : x(x), y(y) {}
    int x, y;
};
inline bool operator==(const TestComponent& lhs, const TestComponent& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

struct NonExistingComponent : public Component
{
};

// insert ourselves between System events so the components can be upcast
// to TestComponent. This is so they can be tested.
class MockEntityManagerHelper : public EntityManagerInterface
{
    void informAddComponent(Entity& e, const Component* c) const override
    { this->informAddComponentHelper(e, static_cast<const TestComponent*>(c)); }
    void informRemoveComponent(Entity& e, const Component* c) const override
    { this->informRemoveComponentHelper(e, static_cast<const TestComponent*>(c)); }

    // WARNING: DO NOT CALL THIS FUNCTION - It is required to be implemented
    // by the base class (abstract function) but doesn't do what it is intended
    // to. This is only here to make the compiler happy.
    Entity& createEntity(const char* name) override { Entity("invalid_entity", this); }

public:
    virtual ~MockEntityManagerHelper() {}
    virtual void informAddComponentHelper(Entity&, const TestComponent*) const = 0;
    virtual void informRemoveComponentHelper(Entity&, const TestComponent*) const = 0;
};

struct MockEntityManager : public MockEntityManagerHelper
{
    MOCK_METHOD1(destroyEntity, void(Entity&));
    MOCK_METHOD1(destroyEntities, void(const char*));
    MOCK_METHOD0(destroyAllEntities, void());
    MOCK_CONST_METHOD2(informAddComponentHelper, void(Entity&, const TestComponent*));
    MOCK_CONST_METHOD2(informRemoveComponentHelper, void(Entity&, const TestComponent*));

};

// ----------------------------------------------------------------------------
// tests
// ----------------------------------------------------------------------------

TEST(NAME, CreateEntityWithName)
{
    MockEntityManager em;
    Entity entity("entity1", &em);
    ASSERT_EQ(std::string("entity1"), std::string(entity.getName()));
}

TEST(NAME, AddingAndRemovingComponentsInformsEntityManager)
{
    MockEntityManager em;
    Entity entity("entity", &em);

    EXPECT_CALL(em, informAddComponentHelper(testing::_, testing::Pointee(TestComponent(2, 3))))
        .Times(1);
    EXPECT_CALL(em, informRemoveComponentHelper(testing::_, testing::Pointee(TestComponent(12, 3))))
        .Times(1);

    entity.addComponent<TestComponent>(2, 3);
    entity.getComponent<TestComponent>().x = 12;
    entity.removeComponent<TestComponent>();
}

TEST(NAME, AddingTwoComponentsOfTheSameTypeCausesDeath)
{
    MockEntityManager em;
    Entity entity("entity", &em);

    // uninteresting calls
    EXPECT_CALL(em, informRemoveComponentHelper(testing::_, testing::_));

    // interesting calls
    EXPECT_CALL(em, informAddComponentHelper(testing::_, testing::Pointee(TestComponent(2, 3))))
        .Times(1);

    entity.addComponent<TestComponent>(2, 3);

    ASSERT_DEATH(entity.addComponent<TestComponent>(3, 4), "");
}

TEST(NAME, GettingNonExistingComponentsCausesDeath)
{
    MockEntityManager em;
    Entity entity("entity", &em);

    // uninteresting calls
    EXPECT_CALL(em, informRemoveComponentHelper(testing::_, testing::_));
    EXPECT_CALL(em, informAddComponentHelper(testing::_, testing::_));

    entity.addComponent<TestComponent>(2, 3);
    ASSERT_EQ(TestComponent(2, 3), entity.getComponent<TestComponent>());
    ASSERT_DEATH(entity.getComponent<NonExistingComponent>(), "");
}

TEST(NAME, EntityDestructionInformsEntityManagerAboutComponentRemoval)
{
    MockEntityManager em;
    Entity entity("entity", &em);

    // uninteresting calls
    EXPECT_CALL(em, informAddComponentHelper(testing::_, testing::_));

    // interesint calls
    EXPECT_CALL(em, informRemoveComponentHelper(testing::_, testing::Pointee(TestComponent(2, 4))))
        .Times(1);

    entity.addComponent<TestComponent>(2, 4);
}
