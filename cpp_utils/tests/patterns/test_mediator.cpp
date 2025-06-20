#include "cpp_utils/patterns/Mediator.h"
#include "gmock/gmock.h"
#include <optional>
#include <string>
#include <vector>

using ::testing::NiceMock;

class ChatRoom;

class User {
public:
    virtual ~User() = default;

    virtual auto join() -> void = 0;

    virtual auto leave() -> void = 0;

    virtual auto send(const std::string& message) -> void = 0;

    virtual auto receive(const std::string& message) -> void = 0;

    virtual auto name() const -> std::string = 0;
};

class ChatRoom : public patterns::Mediator<User> {
public:
    ChatRoom(std::string name_)
        : chat_room_name{std::move(name_)}
    {
    }

    auto join(User* new_user) -> void
    {
        addColleague(new_user);
        mediate(new_user, [&](User* user) {
            user->receive(
                std::format("[User {} has joined the room]", new_user->name()));
        });
    }

    auto leave(User* leaving_user)
    {
        mediate(leaving_user, [&](User* user) {
            user->receive(std::format("[User {} has left the room]",
                                      leaving_user->name()));
        });
        removeColleague(leaving_user);
    }

    auto send_message(User* sender, const std::string& message) -> void
    {
        mediate(sender, [&](User* user) {
            user->receive(std::format("[{}] {}", sender->name(), message));
        });
    }

    auto rename(std::string new_name) -> void
    {
        notifyAll([&](User* user) {
            user->receive(std::format("Chat room renamed to '{}'", new_name));
        });
    }

private:
    std::string chat_room_name;
};

class UserImpl : public User {
public:
    UserImpl(ChatRoom& chat_room_, std::string name_)
        : chat_room{chat_room_}
        , user_name{std::move(name_)}
    {
    }

    auto join() -> void override { chat_room.join(this); }

    auto leave() -> void override { chat_room.leave(this); }

    auto send(const std::string& message) -> void override
    {
        chat_room.send_message(this, message);
    }

    auto receive(const std::string& /* message */) -> void override { }

    auto name() const -> std::string override { return user_name; }

private:
    ChatRoom& chat_room;
    std::string user_name;
};

class ColleagueMock : public User {
public:
    MOCK_METHOD(void, join, (), (override));

    MOCK_METHOD(void, leave, (), (override));

    MOCK_METHOD(void, send, (const std::string&), (override));

    MOCK_METHOD(void, receive, (const std::string&), (override));

    MOCK_METHOD(std::string, name, (), (const override));
};

class MediatorFixture : public ::testing::Test {
public:
    ChatRoom mediator{"Some name"};
};

TEST_F(MediatorFixture, notifies_all_colleagues)
{
    NiceMock<ColleagueMock> user1;
    NiceMock<ColleagueMock> user2;
    mediator.join(&user1);
    mediator.join(&user2);
    std::string expected_message{"Chat room renamed to 'New name'"};

    EXPECT_CALL(user1, receive(expected_message));
    EXPECT_CALL(user2, receive(expected_message));

    mediator.rename("New name");
}

TEST_F(MediatorFixture, mediates)
{
    NiceMock<ColleagueMock> user1;
    NiceMock<ColleagueMock> user2;
    UserImpl user3{mediator, "User 3"};
    mediator.join(&user1);
    mediator.join(&user2);
    mediator.join(&user3);
    std::string expected_message{"[User 3] some message"};

    EXPECT_CALL(user1, receive(expected_message));
    EXPECT_CALL(user2, receive(expected_message));

    user3.send("some message");
}

