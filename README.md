## sj_server

    sj_server 是一个简单的游戏服务器。
    基于 libuv 进行搭建。
    包括:
        fight_game_server   战斗游戏服
        link_server         连接服务器
        city_game_server    城镇游戏服
        global_server       全局服务器
        db_server           数据服务器

## fight_game_server

    fight_game_server 是用来管理所有战斗场景的服务器。
    使用 udp 与 client 直连。
    使用 tcp 与 global_server 连接。
    使用帧同步策略同步相同战斗房间内的玩家的操作。
    战斗结束后将游戏结果反馈给全局服务器。

## link_server

    link_server 是用来管理玩家连接的服务器。
    使用 tcp 与 client 直连。
    使用 tcp 与 city_game_server 连接。
    使用 tcp 与 global_server 连接。
    主要用来转发 client 与 server 之间的协议。
    可以用来校验每条链接的数据是否应被处理，自动抛弃恶意数据。

## city_game_server
    
    city_game_server 是用来管理所有城镇场景的服务器。
    使用 tcp 与 link_server 连接。
    使用 tcp 与 global_server 连接。
    使用 tcp 与 db_server 连接。

## global_server

    global_server 是用来管理全局数据的服务器。
    使用 tcp 与 fight_game_server 连接。
    使用 tcp 与 link_server 连接。
    使用 tcp 与 city_game_server 连接。
    使用 tcp 与 db_server 连接。
    兼具聊天服务器与匹配服务器功能。 未来可拆分。

## db_server

    db_server 是用来缓存数据，与数据库直接交互的服务器。
    使用 tcp 与 city_game_server 连接。
    使用 tcp 与 global_server 连接。
