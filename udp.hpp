#pragma once

#include "stream.hpp"
#include "net.hpp"
#include "loop.hpp"

namespace uvpp
{
    class Udp : public handle<uv_udp_t>
    {
    public:
        Udp():
            handle()
        {
            uv_udp_init(uv_default_loop(), get());
        }

        Udp(loop& l):
            handle()
        {
            uv_udp_init(l.get(), get());
        }

        // FIXME: refactor with getaddrinfo
        bool bind(const std::string& ip, int port)
        { 
            ip4_addr addr = to_ip4_addr(ip, port); 
            return uv_udp_bind(get(), reinterpret_cast<sockaddr*>(&addr), 0) == 0;
        }

        bool bind6(const std::string& ip, int port)
        { 
            ip6_addr addr = to_ip6_addr(ip, port); 
            return uv_udp_bind(get(), reinterpret_cast<sockaddr*>(&addr), 0) == 0;
        }

        bool getsockname(bool& ip4, std::string& ip, int& port)
        {
            struct sockaddr_storage addr;
            int len = sizeof(addr);
            if(uv_udp_getsockname(get(), reinterpret_cast<struct sockaddr*>(&addr), &len) == 0)
            {
                ip4 = (addr.ss_family == AF_INET);
                if(ip4)
                {
                    return from_ip4_addr(reinterpret_cast<ip4_addr*>(&addr), ip, port);
                }
                else
                {
                    return from_ip6_addr(reinterpret_cast<ip6_addr*>(&addr), ip, port);
                }
            }
            return false;
        }

        int set_membership(const char* multicast_addr, const char* interface_addr, uv_membership membership)
        {
            return uv_udp_set_membership(get(), multicast_addr, interface_addr, membership);
        }

        int set_multicast_loop(int on)
        {
            return uv_udp_set_multicast_loop(get(), on);
        }

        int set_multicast_ttl(int ttl)
        {
            return uv_udp_set_multicast_ttl(get(), ttl);
        }

        // int set_multicast_interface(const char* interface_addr)
        // {
        //     return uv_udp_set_multicast_interface(get(), interface_addr);
        // }

        int set_broadcast(int on)
        {
            return uv_udp_set_broadcast(get(), on);
        }

        int set_ttl(int ttl)
        {
            return uv_udp_set_ttl(get(), ttl);
        }

        //typedef void (*uv_udp_send_cb)(uv_udp_send_t* req, int status);
        typedef std::function<void(uv_udp_send_t* req, int status)> send_cb_t;
        int send(uv_udp_send_t* req, const uv_buf_t bufs[], unsigned int nbufs, const struct sockaddr* addr, send_cb_t callback)
        {
            callbacks::store(handle<uv_udp_t>::get()->data, uvpp::internal::uv_cid_send, callback);
            return uv_udp_send(req, get(), bufs, nbufs, addr, [](uv_udp_send_t* req, int status)
                {
                    callbacks::invoke<decltype(callback)>(req->handle->data, uvpp::internal::uv_cid_send, req, status);
                });
        }

        // typedef void (*uv_udp_recv_cb)(uv_udp_t* handle,
        //                        ssize_t nread,
        //                        const uv_buf_t* buf,
        //                        const struct sockaddr* addr,
        //                        unsigned flags);
        typedef std::function<void(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)> recv_cb_t;
        int recv_start(uv_alloc_cb alloc_cb, recv_cb_t callback)
        {
            callbacks::store(handle<uv_udp_t>::get()->data, uvpp::internal::uv_cid_send, callback);
            return uv_udp_recv_start(get(), alloc_cb, [](uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned flags)
                {
                    callbacks::invoke<decltype(callback)>(handle->data, uvpp::internal::uv_cid_send, handle, nread, buf, addr, flags);
                });
        }

        int recv_stop()
        {
            return uv_udp_recv_stop(get());
        }
    };
}
