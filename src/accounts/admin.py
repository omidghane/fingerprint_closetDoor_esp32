from django.contrib import admin
from django.contrib.auth.admin import UserAdmin

from django.contrib.auth import get_user_model

from .models import Profile

User = get_user_model()


class CustomUserAdmin(UserAdmin):
    model = User
    list_display = ('username', 'is_staff', 'is_superuser', 'is_active', 'is_personnel')
    list_filter = ('username', 'is_staff', 'is_superuser', 'is_active', 'is_personnel')

    searching_fields = ('username',)

    ordering = ('created_at',)
    fieldsets = [
        ('Authentication', {'fields': ('username', 'password')}),
        ('Permissions', {'fields': ('is_staff', 'is_superuser', 'is_active', 'is_personnel')}),
        ('groups', {'fields': ('groups', 'user_permissions')}),
        ('important dates', {'fields': ('created_at', 'updated_at', 'last_login')}),
    ]

    add_fieldsets = [
        (None, {
            'classes': ('wide',),
            'fields': ('username', 'password1', 'password2', 'is_staff', 'is_superuser', 'is_active'),
        }),
    ]

    readonly_fields = ("created_at", "updated_at", "last_login")


admin.site.register(User, CustomUserAdmin)
admin.site.register(Profile)
